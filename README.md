# Sistem Monitoring Kelembapan Tanah dan Keberadaan Air Berbasis ESP32

## 1. Deskripsi Proyek

Proyek ini merupakan sistem monitoring berbasis Internet of Things (IoT) yang menggunakan mikrokontroler ESP32 untuk membaca kelembapan tanah dan mendeteksi keberadaan air. Informasi hasil pembacaan sensor ditampilkan melalui OLED dan diintegrasikan dengan dashboard Blynk untuk pemantauan.

Repository ini berisi kode program pengambilan data awal sensor dan kode program utama yang mengintegrasikan seluruh komponen sistem.

## 2. Komponen yang Digunakan

- ESP32 sebagai mikrokontroler utama.
- Sensor kelembapan tanah untuk mengukur kondisi kelembapan tanah.
- Sensor air untuk mendeteksi keberadaan air.
- OLED untuk menampilkan informasi status sistem.
- Blynk sebagai platform dashboard monitoring.

## 3. Struktur Repository

```text
iot-monitoring-esp32/
├── README.md
├── 01-pengambilan-data-awal/
│   └── pengambilan_data.ino
├── 02-sistem-utama/
│   └── sistem_utama.ino
└── docs/
    ├── diagram-arsitektur.png
    ├── diagram-blok.png
    └── dokumentasi-pengembangan.pdf
```

## 4. Penjelasan File Program

### A. Pengambilan Data Awal Sensor

File: `01-pengambilan-data-awal/pengambilan_data.ino`

Program ini digunakan untuk melakukan pembacaan awal sensor sebelum integrasi sistem secara keseluruhan. Data hasil pembacaan dapat digunakan untuk mengamati respons sensor dan menentukan rentang nilai pembacaan yang diperlukan dalam pengembangan sistem.

### B. Program Utama Sistem IoT

File: `02-sistem-utama/sistem_utama.ino`

Program ini merupakan implementasi utama sistem monitoring yang mengintegrasikan pembacaan sensor kelembapan tanah, pendeteksian keberadaan air, penentuan status sistem, tampilan informasi pada OLED, serta pengiriman data ke dashboard Blynk.

## 5. Cara Menjalankan Program

1. Instal Arduino IDE.
2. Siapkan board ESP32 pada Arduino IDE.
3. Instal library yang diperlukan sesuai dengan kode program.
4. Hubungkan sensor dan OLED ke ESP32 sesuai diagram rangkaian pada folder `docs`.
5. Buka file program yang ingin dijalankan.
6. Sesuaikan konfigurasi pin dan parameter program dengan rangkaian yang digunakan.
7. Untuk program utama, sesuaikan konfigurasi koneksi Wi-Fi dan Blynk.
8. Hubungkan ESP32 ke komputer, pilih board dan port yang sesuai, kemudian unggah program.

**Catatan:** Jalankan satu program sesuai kebutuhan pengujian. Program pengambilan data awal dan program utama merupakan dua program terpisah, sehingga tidak perlu diunggah secara bersamaan.

## 6. Dokumentasi

Dokumentasi tambahan mengenai arsitektur sistem, diagram blok, dan proses pengembangan tersedia pada folder `docs`.

## 7. Informasi

Repository ini dibuat untuk mendokumentasikan kode program dan pengembangan proyek sistem monitoring IoT berbasis ESP32.
