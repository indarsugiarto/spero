"""
Pendekatan baru: bikin menjadi thread independen
"""

"""
cVL53Handler juga ngurusi MQTT untuk sensor-sensor
"""
import paho.mqtt.publish as publish
import paho.mqtt.client as mqtt

from PyQt5 import QtCore
import time

from .config import *

class vl53thread(QtCore.QObject, mqtt.Client):
    connected = QtCore.pyqtSignal()
    newMsg = QtCore.pyqtSignal(str)
    critical = QtCore.pyqtSignal(str)

    def __init__(self, topic, server=SERVER, port=1883, ttl=60):
        super(vl53thread, self).__init__()
        self.t = topic
        self.s = server
        self.p = port
        self.l = ttl
        self.n = topic.replace("vl53l0x","Sensor-")

    def run(self):
        self.on_connect = self.OnConnect
        self.on_message = self.OnMessage
        self.connect(self.s, self.p, self.l)
        self.loop_start()
        print("[{}] Starting event loop!".format(self.t))

    def stop(self):
        self.loop_stop()
        print("[{}] Stoping event loop".format(self.t))

    def OnConnect(self, client, userdata, flags, rc):
        print("[{}] Connected with result code ".format(self.t)+str(rc))
        print("[{}] Subscribing".format(self.t))
        self.subscribe(self.t)
        self.connected.emit()

    def OnMessage(self, client, userdata, msg):
        pesan = str(msg.payload)
        pesan = pesan.replace("b'","").replace("'","")
        #print("[{}] Msg = {}".format(self.t,pesan))
        val = int(pesan)
        if val < CRITICAL_DIST:
            self.critical.emit(self.n)
            print("[{}] Deteksi collision".format(self.t))
        self.newMsg.emit(pesan)


class cVL53Handler(QtCore.QObject):
    running = False
    finished = QtCore.pyqtSignal()
    newMsg = QtCore.pyqtSignal()
    critical = QtCore.pyqtSignal(str)

    def __init__(self):
        super(cVL53Handler, self).__init__()
        self.sensors = [0 for _ in range(8)]
        self.kolektor = QtCore.QTimer()
        self.kolektor.setInterval(COLLECTING_SENSOR_PERIOD) #setiap 100ms
        self.kolektor.timeout.connect(self.kirimData)
        self.kolektor.start()

        print("[VL53] Mempersiapkan vl53l0x1...")
        self.x1 = vl53thread("vl53l0x1")
        self.x1t = QtCore.QThread()
        self.x1.moveToThread(self.x1t)
        self.x1t.started.connect(self.x1.run)
        self.x1t.finished.connect(self.x1t.deleteLater)
        self.x1t.finished.connect(self.x1.deleteLater)
        self.x1.newMsg.connect(self.datas1)
        self.x1.critical.connect(self.collision)
        self.x1t.start()

        print("[VL53] Mempersiapkan vl53l0x2...")
        self.x2 = vl53thread("vl53l0x2")
        self.x2t = QtCore.QThread()
        self.x2.moveToThread(self.x2t)
        self.x2t.started.connect(self.x2.run)
        self.x2t.finished.connect(self.x2t.deleteLater)
        self.x2t.finished.connect(self.x2.deleteLater)
        self.x2.newMsg.connect(self.datas2)
        self.x2.critical.connect(self.collision)
        self.x2t.start()

        print("[VL53] Mempersiapkan vl53l0x3...")
        self.x3 = vl53thread("vl53l0x3")
        self.x3t = QtCore.QThread()
        self.x3.moveToThread(self.x3t)
        self.x3t.started.connect(self.x3.run)
        self.x3t.finished.connect(self.x3t.deleteLater)
        self.x3t.finished.connect(self.x3.deleteLater)
        self.x3.newMsg.connect(self.datas3)
        self.x3.critical.connect(self.collision)
        self.x3t.start()

        print("[VL53] Mempersiapkan vl53l0x4...")
        self.x4 = vl53thread("vl53l0x4")
        self.x4t = QtCore.QThread()
        self.x4.moveToThread(self.x4t)
        self.x4t.started.connect(self.x4.run)
        self.x4t.finished.connect(self.x4t.deleteLater)
        self.x4t.finished.connect(self.x4.deleteLater)
        self.x4.newMsg.connect(self.datas4)
        self.x4.critical.connect(self.collision)
        self.x4t.start()

        print("[VL53] Mempersiapkan vl53l0x5...")
        self.x5 = vl53thread("vl53l0x5")
        self.x5t = QtCore.QThread()
        self.x5.moveToThread(self.x5t)
        self.x5t.started.connect(self.x5.run)
        self.x5t.finished.connect(self.x5t.deleteLater)
        self.x5t.finished.connect(self.x5.deleteLater)
        self.x5.newMsg.connect(self.datas5)
        self.x5.critical.connect(self.collision)
        self.x5t.start()

        print("[VL53] Mempersiapkan vl53l0x6...")
        self.x6 = vl53thread("vl53l0x6")
        self.x6t = QtCore.QThread()
        self.x6.moveToThread(self.x6t)
        self.x6t.started.connect(self.x6.run)
        self.x6t.finished.connect(self.x6t.deleteLater)
        self.x6t.finished.connect(self.x6.deleteLater)
        self.x6.newMsg.connect(self.datas6)
        self.x6.critical.connect(self.collision)
        self.x6t.start()

        print("[VL53] Mempersiapkan vl53l0x7...")
        self.x7 = vl53thread("vl53l0x7")
        self.x7t = QtCore.QThread()
        self.x7.moveToThread(self.x7t)
        self.x7t.started.connect(self.x7.run)
        self.x7t.finished.connect(self.x7t.deleteLater)
        self.x7t.finished.connect(self.x7.deleteLater)
        self.x7.newMsg.connect(self.datas7)
        self.x7.critical.connect(self.collision)
        self.x7t.start()

        print("[VL53] Mempersiapkan vl53l0x5...")
        self.x8 = vl53thread("vl53l0x8")
        self.x8t = QtCore.QThread()
        self.x8.moveToThread(self.x8t)
        self.x8t.started.connect(self.x8.run)
        self.x8t.finished.connect(self.x8t.deleteLater)
        self.x8t.finished.connect(self.x8.deleteLater)
        self.x8.newMsg.connect(self.datas8)
        self.x8.critical.connect(self.collision)
        self.x8t.start()

    def collision(self, source):
        self.critical.emit("Collision detected by "+source)

    def datas1(self, msg):
        self.sensors[0] = msg

    def datas2(self, msg):
        self.sensors[1] = msg

    def datas3(self, msg):
        self.sensors[2] = msg

    def datas4(self, msg):
        self.sensors[3] = msg

    def datas5(self, msg):
        self.sensors[4] = msg

    def datas6(self, msg):
        self.sensors[5] = msg

    def datas7(self, msg):
        self.sensors[6] = msg

    def datas8(self, msg):
        self.sensors[7] = msg

    def kirimData(self):
        a = self.sensors[0]
        b = self.sensors[1]
        c = self.sensors[2]
        d = self.sensors[3]
        e = self.sensors[4]
        f = self.sensors[5]
        g = self.sensors[6]
        h = self.sensors[7]
        msg = HEADER_SENSOR+",{},{},{},{},{},{},{},{}".format(a,b,c,d,e,f,g,h)
        publish.single(TOPIC_STATUS, msg, hostname=SERVER)
        #print("[VL53] Publishing", msg)

    def run(self):
        print("[cVL53Handler] Starting...")
        self.running = True
        while self.running:
            QtCore.QCoreApplication.processEvents()
        self.finished.emit()

    def stop(self):
        self.running = False

"""
### Berikut untuk testing langsung:

class omniCtrl(QtCore.QObject):
    finished = QtCore.pyqtSignal()
    running = False
    def __init__(self):
        super(omniCtrl, self).__init__()
        self.vl53 = cVL53Handler()
        self.vl53t = QtCore.QThread()
        self.vl53.moveToThread(self.vl53t)
        self.vl53t.started.connect(self.vl53.run)
        self.vl53t.finished.connect(self.vl53t.deleteLater)
        self.vl53t.finished.connect(self.vl53.deleteLater)
        self.vl53t.start()


if __name__=="__main__":
    myPID = os.getpid()
    print("[MAIN] Dijalankan dengan PID-{}".format(myPID))
    app = QtCore.QCoreApplication(sys.argv)
    sub = omniCtrl()
    sys.exit(app.exec_())
"""
