import scipy.io.wavfile as wav
import librosa
import numpy as np


y, s = librosa.load("blender.wav", sr=16000, mono=True)

int16_samples = []

for sample in y:
    new_value = np.int16((32767 - -32768)/(1 - -1)*(sample - 1) + 32767)
    int16_samples.append(new_value)

print(int16_samples[4096:4096+512])