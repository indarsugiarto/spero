from gpiozero import PWMOutputDevice, DigitalOutputDevice
import sys
from time import sleep

pwmPin = [14,23,25,1] 
dirPin = [15,24,8,7]
f = 5000

if len(sys.argv) == 1:
    print("Berikan arah, speed dan lamanya (detik)")
    sys.exit(-1)

speed = float(sys.argv[2])
arah = int(sys.argv[1])
lama = int(sys.argv[3])

zf = list()
pwm = list()

print("Keempat motor dijalankan selama", lama, "detik dengan kecepatan", speed, "%");

for i in range(4):
    zf.append(DigitalOutputDevice(dirPin[i], initial_value = arah))
    pwm.append(PWMOutputDevice(pwmPin[i], initial_value = speed, frequency = f))

sleep(lama)

for i in range(4):
    zf[i].close()
    pwm[i].close()

print("Done...!")

