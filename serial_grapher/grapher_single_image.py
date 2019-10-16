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
# chunk = 4096 # 2^12 samples for buffer


with open(script_path + '/output.txt','r') as graph_data_file:
    graph_data = graph_data_file.read()
lines = graph_data.split(' ')
ys = []
for line in lines:
    if len(line) > 1:
        print(line)
        ys.append(int(line))

chunk = len(ys)
f_vec = samp_rate*np.arange(chunk/2)/chunk
# Plot one settings

sample_scale = np.arange(0, chunk + 1, 8192)
time_scale = np.around(np.linspace(0, chunk*period*1000, num=len(sample_scale)), 2)

ax1.set_xticks(sample_scale)
ax1.set_xticklabels(sample_scale)
ax1.set_ylim(-32768, 32767)
ax1.set(xlabel="t [ms]")

# Plot two settings


ax2.set(xlabel='Frequency [Hz]')
# plt.ylabel('Amplitude [Pa]')
ax2.set_xscale('log')


# Clear lines from plots
if len(ax1.lines) != 0:
    del ax1.lines[0]
    del ax2.lines[0]

# Calc FFT
fft_data = (np.abs(np.fft.fft(ys))[0:int(np.floor(chunk/2))])/chunk
fft_data[1:] = 2*fft_data[1:]

ax2.set_ylim(0, np.max(fft_data))

# Draw 
print(len(f_vec))
print(len(fft_data))

ax1.plot(ys, color='b')
ax2.plot(f_vec, fft_data, color='red')


plt.show()

