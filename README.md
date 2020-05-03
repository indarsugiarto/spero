# SPERO (Support in Pandemic and Epidemic Robot)

Repository untuk project SPERO. Mungkin akan melibatkan beberapa sub-project, tapi saat ini yang sudah terlihat jelas adalah dua bagian ini:
1. spero-gui: berisi source code untuk program GUI (graphical user interface) spero yang akan dijalankan oleh komputer operator
2. spero-ctrl: berisi source code untuk kontrol robot spero yang akan berjalan di raspberry pi 4

Untuk program GUI, kita akan menggunakan framework Qt5 dengan bahasa pemrograman C++ (bisa juga menggunakan PyQt5, tapi mungkin tidak ideal untuk video processing).

Sedangkan untuk program kontrol robot (spero-ctrl), kita akan memanfaatkan ROS melodic dan program yang ditulis dengan Python

Berikut adalah daftar kerjaan yang musti kita kerjakan (TODO list, bisa ditambahkan):
1. Bikin mobile base:
   a. Pengadaakn BLDC dan driver-nya 
   b. Pengujian BLDC dengan driver-nya
   c. Disain rangka dan struktur untuk base
   d. Pembuatan rangka dan struktur base
   e. Pemasangan komponen pada base: BLDCs+drivers, roda, raspi, battery
2. Bikin Struktur Atas (Rak dan penyangganya):
   a. Disain rangka dan struktur atas (rak)
   b. Pembuatan rangka dan struktur atas
   c. Penempatan tablet, camera, dan alarm lamp (kalau ada)
   d. Pengaturan cabling
3. Bikin GUI dan Apps di Android Tablet:
   a. Instalasi libraries and frameworks untuk Raspi
   b. Pembuatan program kontrol 
   c. Pembuatan GUI lengkap dan installer-nya
   d. Pembuatan aplikasi teleconference di Android Tablet
4. Polishing
   a. Pembuatan/pengecatan body robot
   b. Instalasi charging station
