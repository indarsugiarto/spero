#ifndef SPERODEF_H
#define SPERODEF_H

// Berisi definisi-definisi dasar untuk program SPERO
#define CONFIG_FILE_NAME    "spero.conf"
#define PARAM_NAME_ROBOT_IP     "robotIP"
#define DEFAULT_ROBOT_IP        "192.168.1.15"
#define PARAM_NAME_OMNICAM_IP   "omnicamIP"
#define DEFAULT_OMNICAM_IP      "192.168.1.11"
#define PARAM_NAME_FRONTCAM_IP  "frontcamIP"
#define DEFAULT_FRONTCAM_IP     "192.168.1.16"
#define PARAM_NAME_TABLET_IP    "tabletIP"
#define DEFAULT_TABLET_IP       "192.168.1.17"

#define ROBOT_PING_INTERVAL_MS  1000
#define ROBOT_REPLAY_TIMEOUT_MS 2500    // dalam 2 detik harus sudah merespon

#define JS_KIRI         0
#define JS_KANAN        1
#define MAJU            0
#define MUNDUR          1
#define GESER_KIRI      2
#define GESER_KANAN     3
#define PUTAR_KIRI      4
#define PUTAR_KANAN     5
#define TOPIC_JS        "sperojs"
#define TOPIC_STATUS    "sperostatus"
#define HEADER          "JOYSTICK"
#define STOP_CMD        "SHUTDOWN"      // ini perintah untuk SHUTDOWN!!!
#define TEST_RODA_CMD	"TESTRODA"  // untuk test masing-masing roda (harus floating!!!)
#define TEST_BASE       "TESTBASE"  // untuk test maju, mundur, geser, dan putar
#define TEST_TRAY       "TESTTRAY"  // untuk test sistem LED di masing-masing TRAY
#define BRAKE_CMD       "BRAKE"     // untuk menghentikan SPERO secepatnya
#define RELEASE_CMD     "RELEASE"
#define PING_CMD        "PING"      // dikirim via TOPIC_JS
#define PING_REPLY      "PONG"      // dikirim via TOPIC_STATUS
#define RESET_SENSOR    "RESET_SENSOR"  // kirim buat reset sensor tanpa payload

// berhubungan dengan data MQTT yang dikirim dari robot
#define SENSOR_HEADER   "SENSOR"        // payload ada 8 (karena 8 sensor)
#define COLLISION_HEADER "COLLISION"    // payload hanya satu, yaitu angka "0" (sudah di-unlocked) atau "1" (spero di-locked)
#define BATTERY_HEADER  "BATTERY"       // payload dua, yaitu "kapasitas battery" dan "warning on/off"
#define PING_HEADER     PING_CMD
#define TRAY_HEADER     "TRAY"      // dikirim via TOPIC_JS, butuh 2 payload: #tray, kondisi (0 atau 1)
#define CAM_HEADER      "CAM"       // payload cuma 0 atau 1
#define LAMP_HEADER     "LAMP"
#define SERVO_HEADER    "SERVO"
#define TABLET_HEADER   "TABLET"

#define TABLET_UDP_PORT 7000
#define TABLET_BAT_PERIOD   60000   // setiap 1 menit sekali
//#define TABLET_BAT_PERIOD   1000   // setiap 1 menit sekali
#define TABLET_BAT_CMD  "bat"

#define ACCU_MAX    810 // setara dengan 12.8V
#define ACCU_MIN    690 //setara dengan 11V
//#define ACCU_MIN    730 //setara dengan 11V
//#define ACCU_MIN    770 //setara dengan 11V
#define ACCU_THRESHOLD 25 //dalam prosen

#endif // SPERODEF_H
