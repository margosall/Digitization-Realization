import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import pyformulas as pf
import os
import numpy as np
import time


script_path =  os.path.dirname(os.path.realpath(__file__))

fig = plt.figure()

screen = pf.screen(title='Plot')


chans = 1 # 1 channel
samp_rate = 44100 # 44.1kHz sampling rate
period = 1/samp_rate
# chunk = 4096 # 2^12 samples for buffer

ax1 = fig.add_subplot(3,1,1)
ax2 = fig.add_subplot(3,1,3)

# ax1.set_ylim(-32768, 32767)
ax2.set(xlabel='Frequency [Hz]')
# plt.ylabel('Amplitude [Pa]')
ax2.set_xscale('log')

ser = serial.Serial('/dev/ttyUSB1', 1500000)

first_data = ser.readline()

while True:
    while ser.in_waiting:
        try:
            reading = np.array(list(map(int, ser.readline().decode('utf-8').strip().split(' '))))

            chunk = len(reading)
            # print(chunk)

            sample_scale = np.around(np.arange(0, chunk + 1, chunk/6), 1)
            time_scale = np.around(np.linspace(0, chunk*period*1000, num=len(sample_scale)), 2)

            ax1.set_xticks(sample_scale)
            ax1.set_xticklabels(sample_scale)
            ax1.set(xlabel="sample")
            
            # ax1.set_xticklabels(time_scale)
            # ax1.set(xlabel="t [ms]")
            ax1.set_xlim(0, chunk)


            f_vec = samp_rate*np.arange(chunk/2)/chunk

            # Calc FFT
            fft_data = (np.abs( np.fft.fft(reading) )[0:int(np.floor(chunk/2))])/chunk
            fft_data[1:] = 2*fft_data[1:]

            # plt.plot(sample_numbers, reading)
            if len(ax1.lines) != 0:
                del ax1.lines[0]
                del ax2.lines[0]

            ax1.plot(reading, color='b')
            # print(reading)
            ax2.plot(f_vec, fft_data)

            fig.canvas.draw()

            image = np.fromstring(fig.canvas.tostring_rgb(), dtype=np.uint8, sep='')
            image = image.reshape(fig.canvas.get_width_height()[::-1] + (3,))
            screen.update(image)

        except KeyboardInterrupt:
            ser.close()
        
        except Exception as e:
            print(e)
            time.sleep(0.1)
            
