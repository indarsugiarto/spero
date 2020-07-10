import time

def getTime():
    t = time.strftime("%d-%m-%Y_%H:%M:%S")
    return t

def addLog(filename,msg):
    with open(filename, "a") as fid:
        fid.write("[{}] {}\n".format(getTime(), msg))



