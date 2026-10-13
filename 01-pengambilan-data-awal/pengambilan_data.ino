
#define SOIL_PIN 33
#define WATER_PIN 32

const int SAMPLE = 10;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("===== DRAINSAFE CALIBRATION =====");
  Serial.println("Ambil data 10 kali lalu hitung rata-rata.");
  Serial.println();
}


void loop() {

  long soilTotal = 0;
  long waterTotal = 0;

  for (int i = 0; i < SAMPLE; i++) {
    soilTotal += analogRead(SOIL_PIN);
    waterTotal += analogRead(WATER_PIN);
    delay(100);
  }

  int soilAvg = soilTotal / SAMPLE;
  int waterAvg = waterTotal / SAMPLE;

  Serial.println("===== HASIL KALIBRASI =====");
  Serial.print("Soil Avg : ");
  Serial.println(soilAvg);
  Serial.print("Water Avg: ");
  Serial.println(waterAvg);
  Serial.println("------------------------");
  Serial.println("Kalibrasi selesai. Tekan EN untuk ulang.");

  while (true) {
    delay(1000);
  }
}
