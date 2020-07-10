"""
Kesimpulan: subscribe.callback ternyata blocking...
"""


import os, sys
"""
cVL53Handler juga ngurusi MQTT untuk sensor-sensor
"""
import paho.mqtt.subscribe as subscribe
import paho.mqtt.publish as publish

from PyQt5 import QtCore
import time
#from gpiozero import DigitalOutputDevice

#from .config import *

class cVL53Handler(QtCore.QObject):
    running = False
    finished = QtCore.pyqtSignal()
    newMsg = QtCore.pyqtSignal()

    def __init__(self):
        super(cVL53Handler, self).__init__()
        #self.pwr = DigitalOutputDevice(RELAY1)
        print("[VL53] Turning on Powers...")
        #self.pwr.off()	#off justru on :)
        self.sensors = [0 for _ in range(8)]
        print("Subscribing ke vl53l0x1...")
        subscribe.callback(self.OnMessage1, "vl53l0x1",hostname="iot.petra.ac.id")
        print("Subscribing ke vl53l0x2...")
        subscribe.callback(self.OnMessage2, "vl53l0x2",hostname="iot.petra.ac.id")
        subscribe.callback(self.OnMessage3, "vl53l0x3",hostname="iot.petra.ac.id")
        subscribe.callback(self.OnMessage4, "vl53l0x4",hostname="iot.petra.ac.id")
        subscribe.callback(self.OnMessage5, "vl53l0x5",hostname="iot.petra.ac.id")
        subscribe.callback(self.OnMessage6, "vl53l0x6",hostname="iot.petra.ac.id")
        subscribe.callback(self.OnMessage7, "vl53l0x7",hostname="iot.petra.ac.id")
        print("Subscribing ke vl53l0x8...")
        subscribe.callback(self.OnMessage8, "vl53l0x8",hostname="iot.petra.ac.id")
        self.timer = QtCore.QTimer()
        self.timer.setInterval(100) #setiap 100ms
        self.timer.timeout.connect(self.kirimData)
        self.timer.start()

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
        publish(TOPIC_STATUS, msg)
        print("[VL53] Publishing", msg)

    def OnMessage1(self, client, userdata, msg):
        pesan = str(msg.payload)
        self.sensors[0] = pesan
        #self.newMsg.emit(pesan)

    def OnMessage2(self, client, userdata, msg):
        pesan = str(msg.payload)
        self.sensors[1] = pesan

    def OnMessage3(self, client, userdata, msg):
        pesan = str(msg.payload)
        self.sensors[2] = pesan

    def OnMessage4(self, client, userdata, msg):
        pesan = str(msg.payload)
        self.sensors[3] = pesan

    def OnMessage5(self, client, userdata, msg):
        pesan = str(msg.payload)
        self.sensors[4] = pesan

    def OnMessage6(self, client, userdata, msg):
        pesan = str(msg.payload)
        self.sensors[5] = pesan

    def OnMessage7(self, client, userdata, msg):
        pesan = str(msg.payload)
        self.sensors[6] = pesan

    def OnMessage8(self, client, userdata, msg):
        pesan = str(msg.payload)
        self.sensors[7] = pesan

    def run(self):
        print("[cVL53Handler] Starting...")
        self.running = True
        while self.running:
            QtCore.QCoreApplication.processEvents()
        #kemudian deinisialisasi relay power supply
        print("[VL53] Turning off powers...")
        #self.pwr.on()
        #terakhir, broadcast info kalau sudah terminated
        self.finished.emit()

    def stop(self):
        self.running = False

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
