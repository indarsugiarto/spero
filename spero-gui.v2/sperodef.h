#ifndef SPERODEF_H
#define SPERODEF_H

// Berisi definisi-definisi dasar untuk program SPERO
#define CONFIG_FILE_NAME    "spero.conf"
#define PARAM_NAME_ROBOT_IP  "robotIP"
#define DEFAULT_ROBOT_IP    "192.168.1.15"
#define PARAM_NAME_OMNICAM_IP   "omnicamIP"
#define DEFAULT_OMNICAM_IP  "192.168.1.11"
#define PARAM_NAME_FRONTCAM_IP  "frontcamIP"
#define DEFAULT_FRONTCAM_IP "192.168.1.16"

#define ROBOT_PING_INTERVAL_MS  1000

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


#endif // SPERODEF_H
