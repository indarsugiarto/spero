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

class max471thread(QtCore.QObject, mqtt.Client):
    connected = QtCore.pyqtSignal()
    newMsg = QtCore.pyqtSignal(str)
    def __init__(self, topic, server=SERVER, port=1883, ttl=60):
        super(max471thread, self).__init__()
        self.t = topic
        self.s = server
        self.p = port
        self.l = ttl

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
        print("[{}] Msg = {}".format(self.t,pesan))
        self.newMsg.emit(pesan)


class cMAX471Handler(QtCore.QObject):
    finished = QtCore.pyqtSignal()
    newMsg = QtCore.pyqtSignal()

    def __init__(self):
        super(cMAX471Handler, self).__init__()

        print("[MAX471] Mempersiapkan max471...")
        self.x1 = max471thread("max471")
        self.x1t = QtCore.QThread()
        self.x1.moveToThread(self.x1t)
        self.x1t.started.connect(self.x1.run)
        self.x1t.finished.connect(self.x1t.deleteLater)
        self.x1t.finished.connect(self.x1.deleteLater)
        self.x1.newMsg.connect(self.datas1)
        self.x1t.start()


    def datas1(self, msg):
        val = int(msg)
        if val <= BAT_THRESHOLD:
           warn = 1
        else:
           warn = 0
        #konversi ke persen:
        #y = mx+b
        #100 = m*BAT_MAX + b
        #0 = m*BAT_MIN +b
        #100 = (BAT_MAX-BAT_MIN)*m
        m = 100/(BAT_MAX-BAT_MIN)
        b = 100 - m*BAT_MAX
        pct = m*val + b

        msg = BATTERY_HEADER+",{},{}".format(pct,warn)
        publish.single(TOPIC_STATUS, msg, hostname=SERVER)
        print("BAT ", msg)
