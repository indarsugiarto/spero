import paho.mqtt.client as mqtt
import sys
from PyQt5 import QtCore

topic = "general/text"
host = "127.0.0.1"

class cMqtt(QtCore.QObject):
    finished = QtCore.pyqtSignal()
    newData = QtCore.pyqtSignal(str)
    def __init__(self,topic):
        super(cMqtt, self).__init__()
        self.topic = topic
        self.me = mqtt.Client()
        self.me.on_connect = self.on_connect
        self.me.on_message = self.on_message

    def run(self):
        print("[MQTT] Coba konek ke server...")
        keepalive = 60
        """
        keepalive: maximum period in seconds allowed between communications with the broker. 
                   If no other messages are being exchanged, this controls the rate at which 
                   the client will send ping messages to the broker
        """
        self.me.connect(host, 1883, keepalive)
        self.running = True
        while self.running:
            QtCore.QCoreApplication.processEvents()
            self.me.loop(timeout=0.5)
        self.finished.emit()

    def on_connect(self, client, userdata, flags, rc):
        print("[MQTT] Connected with result code "+str(rc))
        print("[MQTT] Subscribing to", self.topic)
        self.me.subscribe(self.topic)

    def on_message(self, client, userdata, msg):
        print('[MQTT] Topic:"'+msg.topic+'" Message:"'+str(msg.payload)+'"')
        self.newData.emit(str(msg.payload))

class cMain(QtCore.QObject):
    def __init__(self):
        super(cMain, self).__init__()
        self.general = cMqtt("general/text")
        self.generalThread = QtCore.QThread()
        self.general.moveToThread(self.generalThread)
        self.generalThread.started.connect(self.general.run)
        self.general.finished.connect(self.generalThread.quit)
        self.general.finished.connect(self.general.deleteLater)
        self.generalThread.finished.connect(self.generalThread.deleteLater)
        self.general.newData.connect(self.newData)
        self.generalThread.start()

        self.aki = cMqtt("sensor/aki")
        self.akiThread = QtCore.QThread()
        self.aki.moveToThread(self.akiThread)
        self.akiThread.started.connect(self.aki.run)
        self.aki.finished.connect(self.akiThread.quit)
        self.aki.finished.connect(self.aki.deleteLater)
        self.akiThread.finished.connect(self.akiThread.deleteLater)
        self.aki.newData.connect(self.newData)
        self.akiThread.start()

    def newData(self, message):
        stop = str(message).upper().find("STOP")
        if stop >= 0:
            print("Terminating")
            sys.exit(0)

if __name__=="__main__":
    app = QtCore.QCoreApplication(sys.argv)
    main = cMain()
    sys.exit(app.exec_())

