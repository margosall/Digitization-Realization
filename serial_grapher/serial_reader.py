import serial
import os
import numpy as np
import matplotlib.pyplot as plt
from scipy.fftpack import fft

script_path =  os.path.dirname(os.path.realpath(__file__))


with open(script_path + "/data.txt", "w+") as data_file:
    pass # create empty file

ser = serial.Serial('/dev/ttyUSB0', 921600)

while ser.read() != b'\n':
    pass

counter = 1

while True:
    try:
        reading = ser.readline().decode('utf-8')

        values = reading.strip().split(" ")

        lines = []
        counter = 1


        for value in values:
            lines.append(str(counter) + "," + value + "\n")
            counter += 1

        with open(script_path + "/data.txt", "w+") as data_file:
            data_file.writelines(lines)

    except KeyboardInterrupt:
        ser.close()