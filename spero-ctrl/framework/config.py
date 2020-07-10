SERVER        = "127.0.0.1"
PORT          = 1883
TOPIC_JS      = "sperojs"		# harus sama dengan yang ditulis di sisi operator
TOPIC_STATUS  = "sperostatus"
TOPIC_TRAY    = "sperotray"
STOP_CMD      = "STOP"		# MQTT message to stop the program (shutdown)!!!
HEADER_JS     = "JOYSTICK"
HEADER_SENSOR = "SENSOR"
HEADER_TRAY   = "TRAY"
TEST_RODA_CMD = "TESTRODA"
BRAKE_CMD     = "BRAKE"
SHUTDOWN_CMD  = "SHUTDOWN"
LED_ON        = 1
LED_OFF       = 0
BAT_THRESHOLD = 770 	#harus dicari lagi berapa tegangan rendah battery-nya
BAT_MAX       = 810
BAT_MIN       = 690
NET_GUARD_TIME = 3000	#3 detik cukup?
CRITICAL_DIST  = 200     #jarak sensor mendeteksi tabrakan (50mm)
RELEASE_CMD   = "RELEASE"
PING_CMD      = "PING"      # dikirim via TOPIC_JS
PING_REPLY    = "PONG"      # dikirim via TOPIC_STATUS

# berhubungan dengan data MQTT yang dikirim dari robot
SENSOR_HEADER = "SENSOR"         # payload ada 8 (karena 8 sensor)
COLLISION_HEADER = "COLLISION"   # payload hanya satu, yaitu angka "0" (sudah di-unlocked) atau$
BATTERY_HEADER = "BATTERY"       # payload dua, yaitu "kapasitas battery" dan "warning on/off"
PING_HEADER    = PING_CMD
TRAY_HEADER    = "TRAY"      # dikirim via TOPIC_JS, butuh 2 payload: #tray, kondisi (0 atau 1)
CAM_HEADER     = "CAM"       # payload cuma 0 atau 1
LAMP_HEADER    = "LAMP"	     # payload cuma 0 atau 1
SERVO_HEADER   = "SERVO"
TABLET_HEADER  = "TABLET"
TABLET_THRESHOLD = 10		#persen
COLLISION_HEADER = "COLLISION"
RESET_SENSOR_HEADER = "RESET_SENSOR"

# berikut berhubungan dengan pengaturan SERVO
SERVO_MAX      = 0.7
SERVO_MIN      = 0.3
SERVO_FREQ     = 500
SERVO_STEP     = 0.001
SERVO_RATE     = 0.05   #detik

#berikut adalah kode data yang akan dikirim oleh joystick
MAJU          =  0
MUNDUR        =  1
GESER_KIRI    =  2
GESER_KANAN   =  3
PUTAR_KIRI    =  4
PUTAR_KANAN   =  5


#konfigurasi berikut berhubungan dengan PCA9685
PCA_EXIST  = True		# isi True jika dijalankan di ctrl.pcuspero.io
PCA_FREQ   = 1500		# max 1600 buat PCA9685
pinPWM     = {1:15, 2:3, 3:11, 4:7}
pinDIR     = {1:14, 2:2, 3:10, 4:6}	# jika menggunakan PCA juga untuk mengatur arah
arahMAJU   = 0xFFFF
arahMUNDUR = 0x0000
fullSpeed  = 0xFFFF
halfSpeed  = 0x7FFF
noSpeed    = 0x0000
speedCoef  = 0.3

DEF_TEST_RODA_DURATION = 5	# dalam detik

COLLECTING_SENSOR_PERIOD = 100 #dalam ms

#konfigurasi GPIO terutama yang berhubungan dengan relay
RELAY1 = 16	#untuk power 5v (sensor-sensor)
RELAY2 = 20	#untuk charging tablet
RELAY3 = 21	#untuk on/off camera
RELAY4 = 5	#untuk tray-1
RELAY5 = 6	#untuk tray-2
RELAY6 = 13	#untuk tray-3
RELAY7 = 19	#untuk tray-4
RELAY8 = 26	#untuk bangjo
TRAY1  = RELAY4
TRAY2  = RELAY5
TRAY3  = RELAY6
TRAY4  = RELAY7
SERVO  = 11
