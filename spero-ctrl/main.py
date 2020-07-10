#! /usr/bin/python3

"""
Maintainer: Indar Sugiarto (indi@petra.ac.id)
Terakhir update: 24 Juni 2020
"""

from PyQt5 import QtCore
import sys, os, socket
from framework import omniCtrl, config
from gpiozero import DigitalOutputDevice
from framework.config import RELAY1

"""
Sebelum menyalakan power supply buat sensor-sensor, cek dulu apakah
router 192.234.234.234 sudah nyala
"""

#pwr = DigitalOutputDevice(RELAY1)

import time
LOG = "/var/spero/"+time.strftime("%d%m%Y-%H%M%S.log")

def getTime():
    t = time.strftime("%d-%m-%Y_%H:%M:%S")
    return t

timeout = 1
def cek234():
  ulang = True
  while ulang:
    try:
        socket.setdefaulttimeout(timeout)
        socket.socket(socket.AF_INET, socket.SOCK_STREAM).connect(("192.234.234.234", 80))
        print("[INFO] 234 sudah on!")
        ulang = False
    except socket.error as ex:
        #print(ex)
        #print("Kode error:", ex.errno)
        if ex.errno is None:
          print("[INFO] Timeout...")
        elif ex.errno == 111:
          print("[INFO] Port-nya tidak bisa diakses...")
        elif ex.errno == 101:
          print("[INFO] Unreachable, ex: wifi off...")
        time.sleep(timeout)
    except KeyboardInterrupt:
        print("\n\nProgram dihentikan...\n\n")
        sys.exit(-1)
  #lalu nyalakan LED power
  #print("[MAIN] Menyalakan power supply sensor...")
  #pwr = DigitalOutputDevice(RELAY1)
  #pwr.off()

if __name__ == "__main__":
    myPID = os.getpid()
    print("[MAIN] Dijalankan dengan PID-{}".format(myPID))
    with open("/var/spero/spero.pid", "w") as fpid:
        fpid.write("{}\n".format(myPID))

    with open(LOG, "w") as fid:
        fid.write("Initiating log file at "+getTime()+"\n")

    cek234() #tunggu sampai 234 siap
    app = QtCore.QCoreApplication(sys.argv)
    sub = omniCtrl(LOG)
    sys.exit(app.exec_())
