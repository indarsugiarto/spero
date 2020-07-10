from PyQt5 import QtCore

import time

from .config import *
from gpiozero import PWMOutputDevice

"""
Kelas cSERVOHandler akan menghandle servo buat Tablet
"""
class cSERVOHandler(QtCore.QObject):
    running = False
    finished = QtCore.pyqtSignal()
    def __init__(self):
        super(cSERVOHandler, self).__init__()
        self.pwm = PWMOutputDevice(SERVO)
        self.pwm.frequency = SERVO_FREQ
        # letakkan pada posisi tengah
        self.pwm.value = 0.42	# ini posisi pas mendatar
        self.target = 0.42
        self.tick = QtCore.QTimer(self)
        self.tick.setInterval(SERVO_RATE*1000)
        self.tick.timeout.connect(self.move)

    def setValue(self, val):
        b = SERVO_MIN; m = (SERVO_MAX - SERVO_MIN) / 99.0
        self.target = m*float(val) + b
        #self.tick.timeout.connect(self.move)
        self.tick.start()

    def move(self):
        #print("[SERVO] ticking at", time.ctime())
        #v = abs(self.pwm.value - self.target)
        #print("[SERVO] v={}, step={}".format(v,SERVO_STEP))
        if self.pwm.value > self.target:
           self.pwm.value -= SERVO_STEP
        elif self.pwm.value < self.target:
           self.pwm.value += SERVO_STEP
        if abs(self.pwm.value - self.target) < (2*SERVO_STEP):
        #else:
           #v = abs(self.pwm.value - self.target)
           #print("[SERVO] v={}, step={}".format(v,SERVO_STEP))
           self.tick.stop()
           self.target = self.pwm.value


