"""
Mekanisme readyread tidak bisa dipakai disini karena client tidak menggunakan mekanisme 
persistent connection. Jadi ya harus segera dibaca. Hal-hal yang juga bisa bikin error:
- client.disconnected.connect(client.deleteLater)
- client.waitForReadyRead()
"""


import sys
from PyQt5 import QtCore
from PyQt5.QtCore import QByteArray, QDataStream, QIODevice
from PyQt5.QtWidgets import QApplication, QDialog
from PyQt5.QtNetwork import QHostAddress, QTcpServer, QTcpSocket
import time

class cServer(QTcpSocket):
    finished = QtCore.pyqtSignal()
    newData = QtCore.pyqtSignal(str)
    running = False
    def __init__(self):
        super().__init__()
        self.tcpServer = QTcpServer(self)
        PORT = 9999
        address = QHostAddress('127.0.0.1')
        self.tcpServer.listen(address, PORT)
        self.tcpServer.newConnection.connect(self.dealCommunication)
        print("[SERVER] Listening on port-", PORT, "...")

    def dealCommunication(self):
        print("[SERVER] Client is connecting...")
        client = self.tcpServer.nextPendingConnection()
        msg = str(client.readAll(),encoding='utf-8')
        self.newData.emit(msg)

    def run(self):
        self.running = True
        while self.running:
            QtCore.QCoreApplication.processEvents()
        print("[SERVER] Terminated!")

    def stop(self):
        self.running = False
        self.finished.emit()

class cMain(QtCore.QObject):
    finished = QtCore.pyqtSignal()
    def __init__(self):
        super(cMain, self).__init__()
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

        #self.runLoop()

    def runLoop(self):
        try:
            while True:
                QtCore.QCoreApplication.processEvents()
        except KeyboardInterrupt:
            print("\n\n[MAIN] Terminating...")
            self.terminate()

    def terminate(self):
        try:
            self.server.stop()
            self.serverThread.wait(1000)
        finally:
            self.finished.emit()
            sys.exit(0)


    def newData(self, data):
        msg = str(data)
        print("[RECV]", msg)
        if msg.upper().find("STOP") >= 0:
            print("[STOP] Will stopping...")
            #time.sleep(1)
            self.terminate()

if __name__=="__main__":
    import sys
    app = QtCore.QCoreApplication(sys.argv)
    main = cMain()
    sys.exit(app.exec_())

