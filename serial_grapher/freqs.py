import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import time


chans = 1 # 1 channel
samp_rate = 44100 # 44.1kHz sampling rate
chunk = 512 # 2^12 samples for buffer

with open("data.py", "r") as data_file:
    data = data_file.readlines()
f_vec = samp_rate*np.arange(chunk/2)/chunk
fft_data = (np.abs(np.fft.fft(data))[0:int(np.floor(chunk/2))])/chunk
fft_data[1:] = 2*fft_data[1:]

# plot
plt.style.use('ggplot')
plt.rcParams['font.size']=18
fig = plt.figure(figsize=(13,8))
ax = fig.add_subplot(111)



def animate(i):
    try:
        with open("data.py", "r") as data_file:
            data = data_file.readlines()

        fft_data = (np.abs(np.fft.fft(data))[0:int(np.floor(chunk/2))])/chunk
        fft_data[1:] = 2*fft_data[1:]
        ax.clear()
        ax.set_ylim(0, 32726)
        plt.xlabel('Frequency [Hz]')
        # plt.ylabel('Amplitude [Pa]')
        ax.set_xscale('log')
        plt.grid(True)
        plt.plot(f_vec,fft_data)
    except KeyboardInterrupt:
        exit()
    except Exception as e:
        print(e)

anim = animation.FuncAnimation(fig, animate, interval=1)

plt.show()