from time import sleep
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)

INPUT_PIN = 17

GPIO.setup(INPUT_PIN, GPIO.IN)

while 1:
	if(GPIO.input(INPUT_PIN)==1):
		print("1")
	else:
		print("0")
	sleep(.1);

