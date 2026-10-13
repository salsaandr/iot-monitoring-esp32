
#define BLYNK_TEMPLATE_ID "ISI TEMPLATE ID-MU"
#define BLYNK_TEMPLATE_NAME "DRAINSAFE"
#define BLYNK_AUTH_TOKEN "ISI AUTH TOKEN-MU


#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*==============================
        KONFIGURASI WI-FI
==============================*/
char ssid[] = "ISI NAMA WI-FI YANG KAMU GUNAKAN";
char pass[] = "ISI PASSWORD WI-FI YANG KAMU GUNAKAN";

BlynkTimer blynkTimer;

/*==============================
             OLED
==============================*/
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/*==============================
           PIN ESP32
==============================*/
#define SOIL_PIN      33
#define WATER_PIN     32

#define LED_GREEN     18
#define LED_YELLOW    19
#define LED_BLUE       5
#define LED_RED       17

#define BUZZER        23

/*==============================
        KALIBRASI SENSOR
==============================*/
const int SOIL_DRY = 2585;
const int SOIL_WET = 1710;

const int SOIL_THRESHOLD = (SOIL_DRY + SOIL_WET) / 2;

// Histeresis sensor air.
const int WATER_THRESHOLD = 1;

/*==============================
         STATUS TANAMAN
==============================*/
enum PlantStatus {
  HEALTHY,
  NEED_WATER,
  CHECK_DRAINAGE,
  ROOT_ROT_RISK
};

int soilValue = 0;
int waterValue = 0;
int soilPercent = 0;

bool soilWet = false;
bool waterDetected = false;

bool buzzerMuted = false;

PlantStatus currentStatus = HEALTHY;
PlantStatus previousStatus = HEALTHY;

/*==============================
          TIMER SISTEM
==============================*/
unsigned long lastReadTime = 0;
unsigned long waterStartTime = 0;
unsigned long drainageDuration = 0;
unsigned long lastReminderTime = 0;

bool drainageTimerRunning = false;

// true = 10 detik untuk pengujian
// false = 10 menit untuk penggunaan final
const bool TEST_MODE = true;

const unsigned long DRAINAGE_WINDOW =
  TEST_MODE ? 10000UL : 600000UL;

// Pengingat: 1 menit untuk pengujian, 1 jam untuk final
const unsigned long REMINDER_INTERVAL =
  TEST_MODE ? 60000UL : 3600000UL;

const bool DEBUG_DISPLAY = true;

/*==============================
          TIMER BUZZER
==============================*/
const unsigned long BEEP_ON_TIME = 250UL;
const unsigned long BEEP_OFF_TIME = 200UL;

int beepPhase = 0;  // 0=diam, 1=bunyi, 2= jeda
int beepCount = 0;

unsigned long beepPhaseStart = 0;
bool buzzerOutput = false;

/*==============================
       DEKLARASI FUNGSI
==============================*/
void readSoil();
void readWater();
void checkStatus();
void printSensorDebug();
void printStatus();
void updateOLED();
void updateLED();
void startBeep(int count);
void updateBuzzer();
void updateBlynk();
void setBuzzerOutput(bool on);

/*==============================
              SETUP
==============================*/
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED tidak terdeteksi.");
  } else {
    display.clearDisplay();
    display.display();
  }

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  setBuzzerOutput(false);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  blynkTimer.setInterval(1000L, updateBlynk);

  readSoil();
  readWater();
  checkStatus();

  updateOLED();
  updateLED();

  // Hindari bunyi pengingat saat boot.
  previousStatus = currentStatus;
  lastReminderTime = millis();
  updateBuzzer();
}

/*==============================
       BACA SOIL MOISTURE
==============================*/
void readSoil() {
  soilValue = analogRead(SOIL_PIN);

  soilPercent = map(
    soilValue,
    SOIL_DRY,
    SOIL_WET,
    0,
    100
  );

  soilPercent = constrain(soilPercent, 0, 100);
  soilWet = (soilValue <= SOIL_THRESHOLD);
}

/*==============================
         BACA WATER LEVEL
==============================*/
void readWater() {
  waterValue = analogRead(WATER_PIN);
  waterDetected = (waterValue > 0);
}

/*==============================
         DECISION ENGINE
==============================*/
void checkStatus() {
  previousStatus = currentStatus;

  // Tidak ada air di tatakan.
  if (!waterDetected) {
    drainageTimerRunning = false;
    drainageDuration = 0;
    waterStartTime = 0;

    currentStatus = soilWet ? HEALTHY : NEED_WATER;
    return;
  }

  // Ada air dan tanah kering.
  if (!soilWet) {
    drainageTimerRunning = false;
    drainageDuration = 0;
    waterStartTime = 0;

    currentStatus = ROOT_ROT_RISK;
    return;
  }

  // Ada air dan tanah basah: amati drainase.
  if (!drainageTimerRunning) {
    waterStartTime = millis();
    drainageTimerRunning = true;
    drainageDuration = 0;
  }

  drainageDuration = millis() - waterStartTime;

  if (drainageDuration < DRAINAGE_WINDOW) {
    currentStatus = HEALTHY;
  } else {
    currentStatus = CHECK_DRAINAGE;
  }
}

/*==============================
          DEBUG SENSOR
==============================*/
void printSensorDebug() {
  Serial.println("===== DRAINSAFE DEBUG =====");

  Serial.print("Soil Raw       : ");
  Serial.print(soilValue);
  Serial.print(" | ");
  Serial.println(soilWet ? "BASAH" : "KERING");

  Serial.print("Soil Percent   : ");
  Serial.print(soilPercent);
  Serial.println("%");

  Serial.print("Water Raw      : ");
  Serial.print(waterValue);
  Serial.print(" | ");
  Serial.println(waterDetected ? "ADA AIR" : "TIDAK ADA AIR");

  Serial.print("Soil Threshold : ");
  Serial.println(SOIL_THRESHOLD);

  Serial.print("Water Threshold: ");
  Serial.println(WATER_THRESHOLD);

}

/*==============================
          DEBUG STATUS
==============================*/
void printStatus() {
  Serial.print("Status         : ");

  switch (currentStatus) {
    case HEALTHY:
      Serial.println("HEALTHY");
      break;
    case NEED_WATER:
      Serial.println("SIRAM");
      break;
    case CHECK_DRAINAGE:
      Serial.println("CEK DRAINASE");
      break;
    case ROOT_ROT_RISK:
      Serial.println("CEK AKAR");
      break;
  }

  if (drainageTimerRunning) {
    Serial.print("Drain Timer    : ");
    Serial.print(drainageDuration / 1000);
    Serial.println(" detik");
  }

  Serial.print("Buzzer         : ");
  Serial.println(buzzerMuted ? "MUTE" : "AKTIF");

  Serial.println("===========================");
  Serial.println();
}

/*==============================
           OLED DISPLAY
==============================*/
void updateOLED() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);

  switch (currentStatus) {
    case HEALTHY:
      display.println("HEALTHY");
      break;
    case NEED_WATER:
      display.println("SIRAM");
      break;
    case CHECK_DRAINAGE:
      display.println("CEK");
      display.println("DRAINASE");
      break;
    case ROOT_ROT_RISK:
      display.println("CEK AKAR");
      break;
  }

  display.setTextSize(1);

  if (DEBUG_DISPLAY) {
    display.setCursor(0, 42);
    display.print("Soil: ");
    display.print(soilPercent);
    display.print("%");

    display.setCursor(0, 54);

    if (waterDetected) {
      display.print("Drain: ");
      display.print(drainageDuration / 60000);
      display.print("m ");
      display.print((drainageDuration % 60000) / 1000);
      display.print("s");
    } else {
      display.print("Drain: Normal");
    }
  } else {
    display.setCursor(0, 48);

    if (currentStatus == CHECK_DRAINAGE) {
      display.println("Drainase buruk");
    } else if (drainageTimerRunning) {
      display.println("Observasi...");
    } else {
      display.println("Drain normal");
    }
  }

  display.display();
}

/*==============================
           LED STATUS
==============================*/
void updateLED() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_RED, LOW);

  switch (currentStatus) {
    case HEALTHY:
      digitalWrite(LED_GREEN, HIGH);
      break;
    case NEED_WATER:
      digitalWrite(LED_YELLOW, HIGH);
      break;
    case CHECK_DRAINAGE:
      digitalWrite(LED_BLUE, HIGH);
      break;
    case ROOT_ROT_RISK:
      digitalWrite(LED_RED, HIGH);
      break;
  }
}

/*==============================
         KONTROL OUTPUT BUZZER
==============================*/
void setBuzzerOutput(bool on) {
  buzzerOutput = on;
  digitalWrite(BUZZER, on ? HIGH : LOW);
}

/* Mulai pola bunyi tanpa delay().
   count=1 untuk satu beep, count=2 untuk dua beep. */
void startBeep(int count) {
  if (buzzerMuted || count <= 0) {
    return;
  }

  beepCount = count;
  beepPhase = 1;
  beepPhaseStart = millis();
  setBuzzerOutput(true);
}

/*==============================
             BUZZER
==============================*/
void updateBuzzer() {
  unsigned long now = millis();

  // Mute selalu diprioritaskan.
  if (buzzerMuted) {
    beepPhase = 0;
    beepCount = 0;
    setBuzzerOutput(false);
    return;
  }

  // Risiko akar: bunyi terus selama status ini aktif.
  if (currentStatus == ROOT_ROT_RISK) {
    beepPhase = 0;
    beepCount = 0;
    setBuzzerOutput(true);
    return;
  }

  // Status lain tidak memakai bunyi kontinu.
  setBuzzerOutput(false);

  // Healthy: tidak ada pengingat.
  if (currentStatus == HEALTHY) {
    beepPhase = 0;
    beepCount = 0;
    lastReminderTime = now;
    return;
  }

  int requiredBeeps =
    (currentStatus == CHECK_DRAINAGE) ? 2 : 1;

  // Status baru: mulai bunyi satu kali.
  if (previousStatus != currentStatus && beepPhase == 0) {
    startBeep(requiredBeeps);
    return;
  }

  // Jangan memulai pengingat baru saat pola beep masih berlangsung.
  if (beepPhase == 0 &&
      now - lastReminderTime >= REMINDER_INTERVAL) {
    startBeep(requiredBeeps);
    return;
  }

  // Fase 1: akhiri bunyi setelah durasi ON.
  if (beepPhase == 1 &&
      now - beepPhaseStart >= BEEP_ON_TIME) {
    setBuzzerOutput(false);
    beepCount--;
    beepPhase = 2;
    beepPhaseStart = now;
    return;
  }

  // Fase 2: jeda, lalu beep berikutnya bila diperlukan.
  if (beepPhase == 2 &&
      now - beepPhaseStart >= BEEP_OFF_TIME) {
    if (beepCount > 0) {
      beepPhase = 1;
      beepPhaseStart = now;
      setBuzzerOutput(true);
    } else {
      beepPhase = 0;
      lastReminderTime = now;
      setBuzzerOutput(false);
    }
  }
}

/*==============================
          INTEGRASI BLYNK
==============================*/
void updateBlynk() {
  if (!Blynk.connected()) {
    return;
  }

  Blynk.virtualWrite(V0, soilPercent);
  Blynk.virtualWrite(V1, waterDetected ? 1 : 0);

  const char* statusText = "Tidak Diketahui";

  switch (currentStatus) {
    case HEALTHY:
      statusText = "Healthy";
      break;
    case NEED_WATER:
      statusText = "Perlu Disiram";
      break;
    case CHECK_DRAINAGE:
      statusText = "Drainase Buruk";
      break;
    case ROOT_ROT_RISK:
      statusText = "Akar Berisiko";
      break;
  }

  Blynk.virtualWrite(V2, statusText);
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V3);
}

BLYNK_WRITE(V3) {
  // V3 = 1: mute, V3 = 0: buzzer aktif.
  buzzerMuted = (param.asInt() == 1);

  if (buzzerMuted) {
    beepPhase = 0;
    beepCount = 0;
    setBuzzerOutput(false);
  } else {
    // Jangan menunggu pengingat berikutnya untuk status bahaya.
    lastReminderTime = millis();

    if (currentStatus == ROOT_ROT_RISK) {
      setBuzzerOutput(true);
    } else {
      // Biarkan updateBuzzer mengatur bunyi status lainnya.
      beepPhase = 0;
      beepCount = 0;
      setBuzzerOutput(false);
    }
  }
}

/*==============================
               LOOP
==============================*/
void loop() {
  Blynk.run();
  blynkTimer.run();

  // Perbarui buzzer secara teratur, tidak hanya saat sensor dibaca.
  updateBuzzer();

  if (millis() - lastReadTime >= 500UL) {
    lastReadTime = millis();

    readSoil();
    readWater();
    checkStatus();

    updateOLED();
    updateLED();

    printSensorDebug();
    printStatus();
  }
}
