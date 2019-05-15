#!/usr/bin/python3

# Helper to configure Device
# License: GPL
# Author: Andreas Butti

import json
import serial
from struct import pack


def main():
	with open("eeprom.bin", "wb") as file:
		for i in range(0, 128):
			print('.', end='', flush=True)
			ok, result = send_command("r%i\n" % i);
			if result[:2] != "OK":
				print("Failed to read EEPROM at address %d" % i)
				print(">" + result + "<")
				return

			d = int(result[2:])
			file.write(pack("<B", *bytearray([d])))
	print("\nReading finished")


def send_command(cmd):
	try:
		serialPort = serial.Serial("/dev/statelight", 9600, timeout=0.1)
	except BaseException as e:
		serialPort = serial.Serial("/dev/ttyACM0", 9600, timeout=0.1)

	result = ""

	try:
		serialPort.write(cmd.encode())
		serialPort.flush()
		result = serialPort.readline().decode().strip()
	except BaseException as e:
		return False, "Exception " + str(e)

	try:
		serialPort.close()
	except BaseException as e:
		return False, "Exception " + str(e)

	return result == "OK", result


if __name__ == "__main__":
	main()



