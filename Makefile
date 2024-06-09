.PHONY: all build flash

all: build flash

build:
	arduino-cli compile --fqbn esp32:esp32:adafruit_feather_esp32s2

flash:
	arduino-cli upload --fqbn esp32:esp32:adafruit_feather_esp32s2 -p /dev/tty.usbmodem1101
