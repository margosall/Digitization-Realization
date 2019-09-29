import serial
import os
script_path =  os.path.dirname(os.path.realpath(__file__))


ser = serial.Serial('/dev/ttyUSB0', 921600)

while ser.read() != b'\n':
    pass



while True:
    try:
        reading = ser.readline().decode('utf-8')

        print(reading)

    except KeyboardInterrupt:
        ser.close()