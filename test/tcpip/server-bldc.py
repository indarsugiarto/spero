#!/usr/bin/python3

"""
Mekanisme readyread tidak bisa dipakai disini karena client tidak menggunakan mekanisme 
persistent connection. Jadi ya harus segera dibaca. Hal-hal yang juga bisa bikin error:
- client.disconnected.connect(client.deleteLater)
- client.waitForReadyRead()

Untuk percobaan, digunakan dua tombol joystick aja:
Yang kiri: maju, mundur, strive-left, strive-right
Yang kanan: rotate left, rotate right
Jadi belum ada gerak serong
"""


import sys
from PyQt5 import QtCore
from PyQt5.QtCore import QByteArray, QDataStream, QIODevice
from PyQt5.QtWidgets import QApplication, QDialog
from PyQt5.QtNetwork import QHostAddress, QTcpServer, QTcpSocket
import time

"""
Konfigurasi motor-raspi
motor-1: front-right
motor-2: front-left
motor-3: rear-right
motor-4: rear-left
"""

"""
Dari cpp header file:
#define JS_KIRI         0
#define JS_KANAN        1
#define MAJU            0
#define MUNDUR          1
#define GESER_KIRI      2
#define GESER_KANAN     3
#define PUTAR_KIRI      4
#define PUTAR_KANAN     5
#define HEADER          "OMNI"
"""
JS_KIRI        = 0
JS_KANAN       = 1
MAJU           = "0"
MUNDUR         = "1"
GESER_KIRI     = "2"
GESER_KANAN    = "3"
PUTAR_KIRI     = "4"
PUTAR_KANAN    = "5"
HEADER         = "OMNI"     # command dari joystick harus mengandung HEADER ini


from gpiozero import DigitalOutputDevice, PWMOutputDevice
dirPin = [15,24,8,7]    # m1, m2, m3, m4 lihat konfigurasi di atas
pwmPin = [14,23,25,1]   # m1, m2, m3, m4
freq   = 5000
PORT   = 9999

class cServer(QTcpSocket):
    finished = QtCore.pyqtSignal()
    newData = QtCore.pyqtSignal(str)
    running = False
    def __init__(self):
        super().__init__()
        self.tcpServer = QTcpServer(self)
        address = QHostAddress('127.0.0.1')
        self.tcpServer.listen(address, PORT)
        self.tcpServer.newConnection.connect(self.dealCommunication)
        print("[SERVER] Listening on port-", PORT, "...")

    def dealCommunication(self):
        print("[SERVER] Client is connecting...")
        self.client = self.tcpServer.nextPendingConnection()
        self.client.readyRead.connect(self.getData)
        self.client.disconnected.connect(self.client.deleteLater)

    def getData(self):
        msg = str(self.client.readAll(),encoding='utf-8')
        self.newData.emit(msg)

    def run(self):
        self.running = True
        while self.running:
            QtCore.QCoreApplication.processEvents()
        print("[SERVER] Terminated!")

    def stop(self):
        self.running = False
        self.finished.emit()

"""
cBLDC akan mengatur masing-masing motor BLDC secara individu
"""
MAX_SPEED = 0.5
class cBLDC(QtCore.QObject):
    def __init__(self, pinArah, pinPWM):
        super(cBLDC, self).__init__()
        self.zf = DigitalOutputDevice(pinArah)
        self.pwm = PWMOutputDevice(pinPWM)
    def setDirection(self, direction):
        self.zf.value = direction
    def setSpeed(self, speed):
        self.pwm.value = speed
    def accellerate(self):
        """
        buat kecepatan meningkat sampai maksimum,
        tapi untuk sekarang hanya dibuat kecematan konstan
        """
        self.pwm.value = MAX_SPEED
    def decellerate(self):
        """
        buat kecepatannya menurun sampai berhenti,
        tapi sekarang dibuat konstan dulu
        """
        self.pwm.value = 0.0
    def hold(self):
        """
        digunakan untuk menghentikan/menahan nilai pwm ketika motor di accelerate/decellerate
        """
    def close(self):
        """
        digunakan untuk closing pin gpiozero. Apakah perlu dicek kecepatan saat itu?
        """
        # this is abrupt stop:
        self.pwm.value = 0
        self.pwm.close()
        self.zf.close()

"""
cOmni mengatur gerakan dari keempat motor BLDC
"""
class cOmni(QtCore.QObject):
    """
    TODO: buat running di thread sendiri
    """
    finished = QtCore.pyqtSignal()
    def __init__(self):
        super(cOmni, self).__init__()
        self.M1 = cBLDC(dirPin[0], pwmPin[0])
        self.M2 = cBLDC(dirPin[1], pwmPin[1])
        self.M3 = cBLDC(dirPin[2], pwmPin[2])
        self.M4 = cBLDC(dirPin[3], pwmPin[3])
        self.M = [self.M1, self.M2, self.M3, self.M4] # dibuat dalam group
        self.running = True
    def giveCmd(self, cmd):
        """
        giveCmd() adalah slot yang dipanggil dari cMain nanti
        di dalamnya ada perhitungan kinematik
        Note: ada kemungkinan data yang dikirim dobel (header lebih dari satu)
        """
        if cmd.count(HEADER) == 1: #jika ada heder dobel, cueking aja...
            l = cmd.split(",")
            if len(l) == 3:
                valid = True
                if l[1] == MAJU:
                    """
                    gerakan maju
                    """
                    print("[CMD] Maju")
                    self.M1.setDirection(MAJU)
                    self.M3.setDirection(MAJU)
                    self.M2.setDirection(MUNDUR)
                    self.M4.setDirection(MUNDUR)
                elif l[1] == MUNDUR:
                    """
                    gerakan mundur
                    """
                    print("[CMD] Mundur")
                    self.M1.setDirection(MUNDUR)
                    self.M3.setDirection(MUNDUR)
                    self.M2.setDirection(MAJU)
                    self.M4.setDirection(MAJU)
                elif l[1] == GESER_KIRI:
                    """
                    geser kiri: M1,M3 keluar, M2, M4 masuk
                    """
                    print("[CMD] Geser Kiri")
                    self.M1.setDirection(MAJU)
                    self.M3.setDirection(MUNDUR)
                    self.M2.setDirection(MAJU)
                    self.M4.setDirection(MUNDUR)
                elif l[1] == GESER_KANAN:
                    """
                    geser kanan: M1, M3 masuk, M2, M4 keluar
                    """
                    print("[CMD] Geser Kanan")
                    self.M1.setDirection(MUNDUR)
                    self.M3.setDirection(MAJU)
                    self.M2.setDirection(MUNDUR)
                    self.M4.setDirection(MAJU)
                elif l[1] == PUTAR_KIRI:
                    """
                    putar kiri: M1,M3 maju M2,M4 maju
                    """
                    print("[CMD] Putar Kiri")
                    for m in self.M:
                        m.setDirection(MAJU)
                elif l[1] == PUTAR_KANAN:
                    """
                    putar kanan
                    """
                    print("[CMD] Putar Kanan")
                    for m in self.M:
                        m.setDirection(MUNDUR)
                else:
                    print("[INFO] Perintah tidak dikenal...")
                    valid = False
                if valid:
                    for m in self.M:
                        m.setSpeed(float(l[2]))
    def run(self):
        while self.running:
            QtCore.QCoreApplication.processEvents()
        # lalu matikan semua pin yang dipakai
        for m in self.M:
            m.close()

    def stop(self):
        self.running = False
        self.finished.emit()
 
class cMain(QtCore.QObject):
    finished = QtCore.pyqtSignal()

    def __init__(self):
        super(cMain, self).__init__()
        self.initBLDC()
        self.initServer()
        #self.runLoop()

    def initServer(self):
        print("[MAIN] Activating server...")
        self.server = cServer()
        self.serverThread = QtCore.QThread()
        self.server.moveToThread(self.serverThread)
        self.serverThread.started.connect(self.server.run)
        self.server.finished.connect(self.serverThread.quit)
        self.server.finished.connect(self.server.deleteLater)
        self.serverThread.finished.connect(self.serverThread.deleteLater)
        self.server.newData.connect(self.newData)
        self.serverThread.start()
        print("[MAIN] Send STOP command to stop...")

    def initBLDC(self):
        """
        TODO: create thread for each BLDC motor
        """
        print("[MAIN] Preparing all BLDCs...")
        self.omni = cOmni()
        self.omniThread = QtCore.QThread()
        self.omni.moveToThread(self.omniThread)
        self.omniThread.started.connect(self.omni.run)
        self.omni.finished.connect(self.omniThread.quit)
        self.omni.finished.connect(self.omni.deleteLater)
        self.omniThread.finished.connect(self.omniThread.deleteLater)
        self.omniThread.start()


    def runLoop(self):
        try:
            while True:
                QtCore.QCoreApplication.processEvents()
        except KeyboardInterrupt:
            print("\n\n[MAIN] Terminating...")
            self.terminate()

    def terminate(self):
        try:
            # terminate the omnibot controller
            self.omni.stop()
            self.omniThread.wait(1000)   # memastikan aja kalau mati gracefully
            # terminate the server
            self.server.stop()
            self.serverThread.wait(1000)
        finally:
            self.finished.emit()
            #sys.exit(0)

    def newData(self, data):
        msg = str(data)
        print("[RECV]", msg)
        if msg.upper().find("STOP") >= 0:
            print("[STOP] Will stopping...")
            #time.sleep(1)
            self.terminate()
        elif msg.find(HEADER) >= 0:
            """
            Sintaks: <HEADER>,<MODE>,<speed>
            Ada kemungkinan data BLDC lebih dari satu dikirim di saat yang sama
            """
            # kirimkan data ke omni:
            self.omni.giveCmd(msg)
            """
            m = msg.replace(HEADER,"")
            l = m.split(",")
            if len(l)==4:
                m = int(l[1]); a = int(l[2]); s = float(l[3])
            """

if __name__=="__main__":
    import sys
    if len(sys.argv) > 1:
      if sys.argv[1] == "-h" or sys.argv[1] == "help" or sys.argv[1] == "--help":
        print("Gunakan netcat 127.0.0.1 9999 lalu issue-kan STOP dan q untuk berhenti")
        sys.exit(0)
    app = QtCore.QCoreApplication(sys.argv)
    main = cMain()
    main.finished.connect(app.quit)
    sys.exit(app.exec_())

