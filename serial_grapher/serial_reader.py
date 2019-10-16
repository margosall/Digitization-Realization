import serial
import os
import numpy as np
import matplotlib.pyplot as plt

script_path =  os.path.dirname(os.path.realpath(__file__))

ser = serial.Serial('/dev/ttyUSB1', 4000000)

counter = 1

while True:
    while ser.in_waiting:
        try:
            reading = ser.readline().decode('utf-8')


            values = reading.strip().split(" ")

            values = [value + "\n" for value in values]

            with open(script_path + "/data.txt", "w") as data_file:
                data_file.writelines(values)

        except KeyboardInterrupt:
            ser.close()