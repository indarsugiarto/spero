from PyQt5 import QtCore
import time
from time import sleep

from gpiozero import DigitalOutputDevice, PWMOutputDevice
import paho.mqtt.publish as publish
import paho.mqtt.client as mqtt
from .config import *
from .js import cJSHandler
from .pca import cPCAHandler
from .vl53 import cVL53Handler
#from .tray import cTRHandler
from .servo import cSERVOHandler
from .max471 import cMAX471Handler
from .net import cNETHandler
from .logging import addLog


class omniCtrl(QtCore.QObject):
    finished = QtCore.pyqtSignal()
    running = False
    def __init__(self, logfile):
        super(omniCtrl, self).__init__()
        self.log = logfile
        #konfigurasi GPIO
        # setup tray dan matikan
        self.tray = [None for _ in range(4)]
        self.tray[0] = DigitalOutputDevice(RELAY4); self.tray[0].on()
        self.tray[1] = DigitalOutputDevice(RELAY5); self.tray[1].on()
        self.tray[2] = DigitalOutputDevice(RELAY6); self.tray[2].on()
        self.tray[3] = DigitalOutputDevice(RELAY7); self.tray[3].on()
        self.senspwr = DigitalOutputDevice(RELAY1)
        self.senspwr.off()
        self.lamp = DigitalOutputDevice(RELAY8); self.lamp.off()
        self.cam = DigitalOutputDevice(RELAY3); self.cam.off()  #note: power untuk servo ikut ini juga!

        self.locked = False	#kondisi locked bisa terjadi karena collision atau disconnected

        #untuk mekanisme charging tablet
        self.tablet_is_charging = False
        self.tablet = DigitalOutputDevice(RELAY2); self.tablet.on() #matikan dulu

        msg = "[OMNI] Persiapan penerima data joystick..."
        addLog(self.log, msg)
        self.js = cJSHandler()
        self.jst = QtCore.QThread()
        self.js.moveToThread(self.jst)
        self.jst.started.connect(self.js.run)
        self.jst.finished.connect(self.jst.deleteLater)
        self.jst.finished.connect(self.js.deleteLater)
        self.js.newMsg.connect(self.newMsg) #untuk memantau perintah STOP

        #mekanisme melindungi spero dari kehilangan koneksi
        self.netGuard = cNETHandler()
        self.netGuardt = QtCore.QThread()
        self.netGuard.moveToThread(self.netGuardt)
        self.netGuardt.started.connect(self.netGuard.run)
        self.netGuardt.finished.connect(self.netGuardt.deleteLater)
        self.netGuardt.finished.connect(self.netGuard.deleteLater)
        self.netGuard.critical.connect(self.critical)
        self.netGuard.aman.connect(self.relax)
        self.js.newMsg.connect(self.netGuard.newMsg)	#kirim juga "ping" ke net guardian

        msg = "[OMNI] Persiapan servo driver tablet..."
        addLog(self.log, msg)
        self.servo = cSERVOHandler()

        msg = "[OMNI] Persiapan modul kendali PCA..."
        addLog(self.log, msg)
        self.pca = cPCAHandler(self.log)
        self.pcat = QtCore.QThread()
        self.pca.moveToThread(self.pcat)
        self.pcat.started.connect(self.pca.run)
        self.pcat.finished.connect(self.pcat.deleteLater)
        self.pcat.finished.connect(self.pca.deleteLater)
        self.js.newMsg.connect(self.pca.newMsg)

        msg = "[OMNI] Persiapan sensor..."
        addLog(self.log, msg)
        self.vl53 = cVL53Handler()
        self.vl53t = QtCore.QThread()
        self.vl53.moveToThread(self.vl53t)
        self.vl53t.started.connect(self.vl53.run)
        self.vl53t.finished.connect(self.vl53t.deleteLater)
        self.vl53t.finished.connect(self.vl53.deleteLater)
        self.vl53.critical.connect(self.critical)

        msg = "[OMNI] Persiapan max471..."
        addLog(self.log, msg)
        self.m471 = cMAX471Handler()


        self.jst.start()
        addLog(self.log,"[OMNI] Running worker threads joystick..")
        self.pcat.start()
        addLog(self.log, "[OMNI] Running worker threads pca...")
        self.vl53t.start()
        addLog(self.log, "[OMNI] Running worker threads vl54...")
        self.netGuardt.start()
        addLog(self.log, "[OMNI] Running worker threads netGuard...")

    def critical(self, code):
        """
        slot critical ini khusus untuk mendeteksi messaging kritis
        """
        self.pca.brake()
        self.pca.lock()
        msg = "[OMNI] "+code
        print(msg)
        addLog(self.log, msg)
        self.locked = True
        if COLLISION_HEADER in code.upper():
            #dari vl53.py, berbunyi: Collision detected by...
            msg = COLLISION_HEADER+",1"
            publish.single(TOPIC_STATUS, msg, hostname=SERVER)

    def unlock(self, code):
        if code=="0":
            self.pca.lock()
        eli code=="1":
            self.pca.unlock()

    def relax(self, code):
        self.locked = False
        self.pca.unlock()
        if "GUARDIAN" not in code:
            msg = COLLISION_HEADER+",0"
            publish.single(TOPIC_STATUS, msg, hostname=SERVER)

    def newMsg(self, msg):
        """
        kelas js akan mengirim data ke newMsg
        """
        msg = msg.replace("b'","").replace("'","")
        msg = "[OMNI] Receiving message: "+msg
        addLog(self.log, msg)
        pesan = msg.upper()
        if STOP_CMD in pesan or SHUTDOWN_CMD in pesan:
            print("[OMNI] Receiving message: "+msg)
            self.js.stop()
            self.jst.quit()
            self.jst.wait()
            print("[OMNI] Stopping...")
            QtCore.QCoreApplication.instance().quit()
        elif PING_CMD in pesan:
            msg = PING_HEADER+","+PING_REPLY
            publish.single(TOPIC_STATUS, msg, hostname=SERVER)
        elif RELEASE_CMD in pesan:
            #cara lama di self.relax, tapi masalah on-off-on-off dst
            #self.relax("Relaxing collision detector")
            lst = pesan.split(",")
            code = lst[1]
            self.unlock(code)
        elif TRAY_HEADER in pesan:
            print(pesan)
            lst = pesan.split(",")
            self.setTray(int(lst[1]),int(lst[2]))
        elif LAMP_HEADER in pesan:
            lst = pesan.split(",")
            if lst[1] == "0":
                self.lamp.on()
            else:
                self.lamp.off()
        elif SERVO_HEADER in pesan:
            lst = pesan.split(",")
            val = int(lst[1])
            self.servo.setValue(val)
        elif RESET_SENSOR_HEADER in pesan:
            self.senspwr.on(); time.sleep(1); self.senspwr.off()
        elif CAM_HEADER in pesan:
            """
            cbCAM di operator GUI difungsikan untuk ini
            sebenarnya sama dengan RESET_SENSOR_HEADER yang dikirim
            ketika pbTest2 ditekan
            """
            lst = pesan.split(",")
            if lst[1]=="0":
                self.senspwr.on()
            elif lst[1]=="1":
                self.senspwr.off()
        elif TABLET_HEADER in pesan:
            lst = pesan.split(",")
            val = int(lst[1])
            if self.tablet_is_charging:
                if val == 100:
                    self.tablet_is_charging = False
                    self.tablet.on()	#matikan charger
            else:
                if val < TABLET_THRESHOLD:
                    self.tablet_is_charging = True
                    self.tablet.off()	#nyalakan charger


    def setTray(self, num, state):
        if state==0:
            self.tray[num-1].on()
        else:
            self.tray[num-1].off()


