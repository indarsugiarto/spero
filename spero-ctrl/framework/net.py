from PyQt5 import QtCore
import time

from .config import *

import board
import busio
import adafruit_pca9685

"""
Kelas cNETHandler digunakan untuk memastikan bahwa robot masih terhubung dengan operator
"""
class cNETHandler(QtCore.QObject):
    critical = QtCore.pyqtSignal(str)	#dikirim jika tidak lagi mendeteksi koneksi dengan operator
    aman = QtCore.pyqtSignal(str)
    running = False
    def __init__(self):
        super(cNETHandler, self).__init__()
        self.timer = QtCore.QTimer(self)
        self.timer.setInterval(NET_GUARD_TIME)
        self.timer.timeout.connect(self.timeout)
        self.timer.start()

    def timeout(self):
        self.critical.emit("Net guardian cannot detect valid connection")

    def newMsg(self,msg):
        #print("net guardian terima msg...")
        msg = msg.replace("b'","").replace("'","")
        if PING_CMD in msg:
            self.timer.start()
            self.aman.emit("Net guardian tersambung lagi...")

    def stop(self):
        self.running = False

    def run(self):
        self.running = True
        while self.running:
            QtCore.QCoreApplication.processEvents()
