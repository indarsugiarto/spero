from PyQt5 import QtCore
import time

from .config import *
from .logging import addLog

import board
import busio
import adafruit_pca9685

"""
Kelas cPCAHandler akan menghandle modul PCA9685
"""
class cPCAHandler(QtCore.QObject):
    running = False
    finished = QtCore.pyqtSignal()
    def __init__(self, logfname):
        super(cPCAHandler, self).__init__()
        self.log = logfname
        self.locked = False
        self.speed = 0	#mencatat kecepatan saat ini
        #kemudian inisialisasi PCA
        if PCA_EXIST:
            self.i2c = busio.I2C(board.SCL, board.SDA)
            self.pca = adafruit_pca9685.PCA9685(self.i2c)
            self.pca.frequency = PCA_FREQ
            #kemudian inisialisasi kecepatan masing-masing roda
            for roda in range(1,5):
                self.pca.channels[pinPWM[roda]].duty_cycle = 0

    def lock(self):
        self.locked = True
        msg = "[PCA] Robot di lock!"
        print(msg)
        addLog(self.log,msg)
        #self.brake()

    def unlock(self):
        self.locked = False
        msg="[PCA] Robot di unlock"
        print(msg)
        addLog(self.log, msg)

    def newMsg(self,msg):
        #newMsg bisa di supply data dari joystick langsung
        #berikutnya, perlu dihitung kinemaika-nya
        if HEADER_JS in msg:
            #print("Type msg:", type(msg))
            #print("[PCA] Menerima pesan: "+msg); return
            #msg = msg.decode()	#convert bytearray to string
            msg = msg.replace("b'"+HEADER_JS+',','')
            msg = msg.replace("'","")
            lst = msg.split(',')
            if len(lst) != 2:
                print("[PCA] Joystick data error")
            else:
                cmd = int(lst[0])
                s = int(float(lst[1]) * 65535)
                if   cmd == MAJU:
                    print("[PCA] Maju, speed = {}".format(s))
                    self.setSpeed(s)
                    self.maju()
                elif cmd == MUNDUR:
                    print("[PCA] Mundur, speed = {}".format(s))
                    self.setSpeed(s)
                    self.mundur()
                elif cmd == GESER_KIRI:
                    print("[PCA] Geser kiri, speed = {}".format(s))
                    self.setSpeed(s)
                    self.serong_kiri()
                elif cmd == GESER_KANAN:
                    print("[PCA] Geser kanan, speed = {}".format(s))
                    self.setSpeed(s)
                    self.serong_kanan()
                elif cmd == PUTAR_KIRI:
                    print("[PCA] Putar kiri, speed = {}".format(s))
                    self.setSpeed(s)
                    self.putar_kiri()
                elif cmd == PUTAR_KANAN:
                    print("[PCA] Putar kanan, speed = {}".format(s))
                    self.setSpeed(s)
                    self.putar_kanan()
                else:
                    print("[PCA] Unknown command!")
        elif TEST_RODA_CMD in msg:
            print("[PCA] Ada request untuk test roda...")
            self.testRoda()
        elif BRAKE_CMD in msg:
            self.brake()
        elif STOP_CMD in msg:
            self.brake()

    def brake(self):
        #ini untuk berhenti mendadak
        msg = "[PCA] Braking the robot..."
        print(msg)
        addLog(self.log,msg)
        #return
        for roda in range(1,5):
            self.pca.channels[pinPWM[roda]].duty_cycle = 0

    def deccelerate(self):
        #ini untuk berhenti secara gradual
        while self.speed > 0:
            self.speed -= 100
            if self.speed < 0:
                self.speed = 0
            for roda in range(1,5):
                self.pca.channels[pinPWM[roda]].duty_cycle = self.speed
            time.sleep(0.1)

    def testRoda(self):
        #digunakan untuk menguji masing-masing roda selama 10 detik
        #modul PCA harus terpasang jika mau memanggil fungsi ini
        print("[PCA] Akan mulai testing roda...")
        for roda in range(1,5):
            print("Testing Roda-{} arah CW kecepatan setengah selama {} detik".format(roda, DEF_TEST_RODA_DURATION))
            if PCA_EXIST:
                self.pca.channels[pinDIR[roda]].duty_cycle = arahMAJU
                self.pca.channels[pinPWM[roda]].duty_cycle = halfSpeed
            time.sleep(DEF_TEST_RODA_DURATION)
            print("Testing Roda-{} arah CW kecepatan penuh selama {} detik".format(roda, DEF_TEST_RODA_DURATION))
            if PCA_EXIST:
                self.pca.channels[pinPWM[roda]].duty_cycle = fullSpeed
            time.sleep(DEF_TEST_RODA_DURATION)
            print("Testing Roda-{} arah CCW kecepatan penuh selama {} detik".format(roda, DEF_TEST_RODA_DURATION))
            if PCA_EXIST:
                self.pca.channels[pinDIR[roda]].duty_cycle = arahMUNDUR
            time.sleep(DEF_TEST_RODA_DURATION)
            print("Testing Roda-{} arah CCW kecepatan setengah selama {} detik".format(roda,DEF_TEST_RODA_DURATION))
            if PCA_EXIST:
                self.pca.channels[pinPWM[roda]].duty_cycle = halfSpeed
            time.sleep(DEF_TEST_RODA_DURATION)
            print("Testing Roda-{} selesai".format(roda))
            if PCA_EXIST:
                self.pca.channels[pinPWM[roda]].duty_cycle = noSpeed
            time.sleep(2)

    def setSpeed(self, newSpeed):
        if newSpeed <= fullSpeed and newSpeed >= noSpeed:
            self.speed = int(newSpeed*speedCoef)

    def applySpeed(self):
        if self.locked is False:
            for roda in range(1,5):
                self.pca.channels[pinPWM[roda]].duty_cycle = self.speed
        else:
            msg = "[PCA] Robot is locked!"
            print(msg)
            addLog(self.log, msg)

    def maju(self):
        self.pca.channels[pinDIR[1]].duty_cycle = self.pca.channels[pinDIR[3]].duty_cycle = arahMAJU
        self.pca.channels[pinDIR[2]].duty_cycle = self.pca.channels[pinDIR[4]].duty_cycle = arahMUNDUR
        self.applySpeed()

    def mundur(self):
        self.pca.channels[pinDIR[1]].duty_cycle = self.pca.channels[pinDIR[3]].duty_cycle = arahMUNDUR
        self.pca.channels[pinDIR[2]].duty_cycle = self.pca.channels[pinDIR[4]].duty_cycle = arahMAJU
        self.applySpeed()

    def serong_kiri(self):
        self.pca.channels[pinDIR[1]].duty_cycle = self.pca.channels[pinDIR[2]].duty_cycle = arahMUNDUR
        self.pca.channels[pinDIR[3]].duty_cycle = self.pca.channels[pinDIR[4]].duty_cycle = arahMAJU
        self.applySpeed()

    def serong_kanan(self):
        self.pca.channels[pinDIR[1]].duty_cycle = self.pca.channels[pinDIR[2]].duty_cycle = arahMAJU
        self.pca.channels[pinDIR[3]].duty_cycle = self.pca.channels[pinDIR[4]].duty_cycle = arahMUNDUR
        self.applySpeed()

    def putar_kiri(self):
        for roda in range(1,5):
            self.pca.channels[pinDIR[roda]].duty_cycle = arahMAJU
        self.applySpeed()

    def putar_kanan(self):
        for roda in range(1,5):
            self.pca.channels[pinDIR[roda]].duty_cycle = arahMUNDUR
        self.applySpeed()

    def run(self):
        print("[cPCAHandler] Starting...")
        self.running = True
        while self.running:
            QtCore.QCoreApplication.processEvents()

        #kemudian deinisialisasi PCA
        if PCA_EXIST:
            self.brake()
            self.pca.deinit()

        #terakhir, broadcast info kalau sudah terminated
        self.finished.emit()

    def stop(self):
        self.running = False



