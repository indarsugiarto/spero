from gpiozero import PWMOutputDevice, OutputDevice
import sys
from time import sleep

pwmPin = 14
dirPin = 15
f = 5000
d = 0
v = 0.0

# Buka akses BLDC
zf  = OutputDevice(dirPin, initial_value = d)
pwm = PWMOutputDevice(pwmPin, initial_value = v, frequency = f)

print("Tekan ctrl-c untuk berhenti!")
pl = [0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0]

try:
    while True:
        zf.value = 0
        for p in pl:
            pwm.value = p; sleep(0.25)
            print("dir = {}, val = {}".format(zf.value, p))
        sleep(1)
        zf.value = 1
        for p in pl:
            pwm.value = p; sleep(0.25)
            print("dir = {}, val = {}".format(zf.value, p))
        sleep(1)
except KeyboardInterrupt:
    v = pwm.value
    while v > 0.0:
        v -= 0.1
        if v < 0.0:
            v = 0.0
        else: 
            if v>=0:
              pwm.value -= v; sleep(0.25)
    pwm.close()
    zf.close()
print("\n\nDone...!")

