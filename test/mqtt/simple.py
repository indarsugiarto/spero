import paho.mqtt.subscribe as subscribe
import sys
from PyQt5 import QtCore

def on_message_print(client, userdata, message):
    print("%s %s" % (message.topic, message.payload))

#subscribe.callback(on_message_print, "paho/test/callback", hostname="mqtt.eclipse.org")

topic = "general/text"
host = "127.0.0.1"

class cMain(QtCore.QObject):
    def __init__(self):
        super(cMain, self).__init__()
        print("[MAIN] Subscribing to", topic)
        subscribe.callback(self.newData, topic, hostname=host)

    def newData(self, client, userdata, message):
        print("[MSG] ", end="")
        print('Topic:"%s" Message:"%s"' % (message.topic, message.payload))
        stop = str(message.payload).upper().find("STOP")
        if stop >= 0:
            print("Terminating")
            sys.exit(0)

if __name__=="__main__":
    app = QtCore.QCoreApplication(sys.argv)
    main = cMain()
    sys.exit(app.exec_())

