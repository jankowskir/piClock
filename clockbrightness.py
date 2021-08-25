import time
import board
import busio
import adafruit_tsl2591
import RPi.GPIO as GPIO
import subprocess
#import os

# Initialize the I2C bus.
i2c = busio.I2C(board.SCL, board.SDA)

#GPIO.setup(18, GPIO.OUT)  # Set GPIO pin 12 to output mode.

#pwm = GPIO.PWM(18, 90)   # Initialize PWM on pwmPin 100Hz frequency2C(board.SCL, board.SDA)
#pwm.start(100) #Start the PWM at 100%

# Initialize the sensor.
sensor = adafruit_tsl2591.TSL2591(i2c)

#maxBrightness = 100
#bedBrightness = 10
#minBrightness = 2

brightness = "NULL"

lightsOn = 200000
lightsBed = 100000
lightsOff = 10000


smooth1 = 0
smooth2 = 0
smooth3 = 0
smoothed = 0

def checkBrightness():
	temp = 0
	temp = sensor.visible
	print("Temp: ",temp)
	if temp > lightsOn:
		temp = lightsOn

#	if temp > lightsOn:
#		return lightsOn
#	elif temp < lightsOn and temp > lightsOff:
#		return lightsBed
#	elif temp < lightsOff:
#		return lightsOff
	return temp

while True:
	i = 0
	smoothed = 0
	while i < 3:
		print (i)
		i += 1
		smoothed = smoothed + checkBrightness()
#	smooth1 = sensor.visible
		print("Smooth",i,": ",smoothed)
		time.sleep(1)

#	smoothed = smoothed + checkBrightness()
#	smooth2 = sensor.visible + smooth1
#	print("Smooth2: ",smoothed)
#	time.sleep(5)

#	smoothed = smoothed + checkBrightness()
#	smooth3 = sensor.visible + smooth2
#	print("Smooth3: ",smoothed)
	reading = smoothed//3
	print("Reading: ",reading)

	if reading >= lightsOn:
		brightness="High"
	elif reading < lightsOn and reading > lightsOff:
		brightness="Med"
	elif reading < lightsOff:
		brightness="Low"


	print("Brightness: ",brightness)
	str1 = "/bin/echo " + brightness + " > /tmp/brightness"
	print("Command: ", str1)
	output = subprocess.call(str1, shell=True)
#	with open('/tmp/brightness', 'w') as file:
#		file.write(brightness)
#		file.write("\n")
#		file.close()

	time.sleep(1)

