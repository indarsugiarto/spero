from PyQt5 import QtCore

import paho.mqtt.client as mqtt
from .config import *

"""
Kelas cJSHandler akan menggunakan MQTT untuk menerima data dengan topic "sperojs"
"""
class cJSHandler(QtCore.QObject, mqtt.Client):
    connected = QtCore.pyqtSignal()
    newMsg = QtCore.pyqtSignal(str)
    def __init__(self, topic=TOPIC_JS, server=SERVER, port=PORT, ttl=60):
        super(cJSHandler, self).__init__()
        self.t = topic
        self.s = server
        self.p = port
        self.l = ttl

    def run(self):
        print("[cJSHandler] Starting...")
        self.on_connect = self.OnConnect
        self.on_message = self.OnMessage
        self.connect(self.s, self.p, self.l)
        self.loop_start()

    def stop(self):
        self.loop_stop()
        self.disconnect()

    def OnConnect(self, client, userdata, flags, rc):
        print("[cJSHandler] Connected with result code "+str(rc))
        print("[cJSHandler] Subscribing to "+self.t)
        self.subscribe(self.t)
        self.connected.emit()

    def OnMessage(self, client, userdata, msg):
        pesan = str(msg.payload)
        self.newMsg.emit(pesan)


