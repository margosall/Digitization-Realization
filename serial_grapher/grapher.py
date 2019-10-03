import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
import os
import numpy as np
from subprocess import Popen, PIPE
import time


script_path =  os.path.dirname(os.path.realpath(__file__))

process = Popen(['python3', 'serial_reader.py'], stdout=PIPE, stderr=PIPE)

fig = plt.figure()
ax1 = fig.add_subplot(3,1,1)
ax2 = fig.add_subplot(3,1,3)

chans = 1 # 1 channel
samp_rate = 44100 # 44.1kHz sampling rate
period = 1/samp_rate
chunk = 2048 # 2^12 samples for buffer
f_vec = samp_rate*np.arange(chunk/2)/chunk

last_modified = [os.path.getmtime(script_path + '/data.txt')]
# print(last_modified)

# Plot one settings

time_scale = np.around(np.linspace(0, chunk*period*1000, num=9), 2)
sample_scale = np.arange(0, chunk + 1, 256)

ax1.set_xticks(sample_scale)
ax1.set_xticklabels(time_scale)
ax1.set_ylim(-32768, 32767)
ax1.set(xlabel="t [ms]")

# Plot two settings


ax2.set(xlabel='Frequency [Hz]')
# plt.ylabel('Amplitude [Pa]')
ax2.set_xscale('log')



def animate(i, last_modified):
    try:
        new_modified = os.path.getmtime(script_path + '/data.txt')
        if last_modified[0] != new_modified:
            # print(last_modified, new_modified)
            last_modified[0] = new_modified
 
            # Update data
            with open(script_path + '/data.txt','r') as graph_data_file:
                graph_data = graph_data_file.read()
            lines = graph_data.split('\n')
            ys = []
            for line in lines:
                if len(line) > 1:
                    ys.append(int(line))

            # Clear lines from plots
            if len(ax1.lines) != 0:
                del ax1.lines[0]
                del ax2.lines[0]

            # Calc FFT
            fft_data = (np.abs(np.fft.fft(ys))[0:int(np.floor(chunk/2))])/chunk
            fft_data[1:] = 2*fft_data[1:]

            ax2.set_ylim(0, np.max(fft_data))

            # Draw 
            ax1.plot(ys, color='b')
            ax2.plot(f_vec, fft_data, color='red')
    except KeyboardInterrupt:
        process.terminate()
        exit()
    except Exception as e:
        print(e)

ani = animation.FuncAnimation(fig, animate, interval=100, fargs=(last_modified,))
plt.show()
process.terminate()
