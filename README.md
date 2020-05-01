# SPERO (Support in Pandemic and Epidemic Robot)

Repository untuk project SPERO. Mungkin akan melibatkan beberapa sub-project, tapi saat ini yang sudah terlihat jelas adalah dua bagian ini:
1. spero-gui: berisi source code untuk program GUI (graphical user interface) spero yang akan dijalankan oleh komputer operator
2. spero-ctrl: berisi source code untuk kontrol robot spero yang akan berjalan di raspberry pi 4

Untuk program GUI, kita akan menggunakan framework Qt5 dengan bahasa pemrograman C++ (bisa juga menggunakan PyQt5, tapi mungkin tidak ideal untuk video processing).

Sedangkan untuk program kontrol robot (spero-ctrl), kita akan memanfaatkan ROS melodic dan program yang ditulis dengan Python
