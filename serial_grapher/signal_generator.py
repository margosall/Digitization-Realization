import pyaudio
import numpy as np
from scipy import signal as sg
import matplotlib.pyplot as plt

p = pyaudio.PyAudio()

defualt_device_index = p.get_default_output_device_info()['index']
print(defualt_device_index)

sampling_rate = 48000
freq = 100
freq2 = 500
freq3 = 1000
samples = sampling_rate
x = np.arange(samples)

y = np.sin(2 * np.pi * freq * x / sampling_rate)
y2 = np.sin(2 * np.pi * freq2 * x / sampling_rate)
y3 = np.sin(2 * np.pi * freq3 * x / sampling_rate)
y = np.add(1 * np.sin(2 * np.pi * freq2 * x / sampling_rate), y)
y = np.subtract(y, y3)

y = y * 0.3

# y = np.add(y, y2)

data =  y.astype(np.float32).tostring()


# plt.plot(x, y)
# plt.show()

stream = p.open(format= pyaudio.paFloat32,
                channels = 1,
                rate = sampling_rate,
                output = True)

while True:
    stream.write(data)

stream.stop_stream()
stream.close()

p.terminate()
