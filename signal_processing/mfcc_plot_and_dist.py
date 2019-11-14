from python_speech_features import mfcc
from python_speech_features import logfbank
import scipy.io.wavfile as wav
import matplotlib.pyplot as plt
import numpy as np
from matplotlib import cm
from scipy.spatial.distance import euclidean

from fastdtw import fastdtw


(rate1, sig1) = wav.read("water.wav")
(rate2, sig2) = wav.read("blender.wav")
(rate3, sig3) = wav.read("humvee.wav")
(rate4, sig4) = wav.read("water.wav")


mfcc_feat1 = mfcc(sig1[:44100], rate1, nfft=2048)
mfcc_feat2 = mfcc(sig2[:44100], rate2, nfft=2048)
mfcc_feat3 = mfcc(sig3[:44100], rate3, nfft=2048)
mfcc_feat4 = mfcc(sig4[44100:2*44100], rate4, nfft=2048)

plt.matshow(mfcc_feat1.T, cmap="Greys_r")
plt.xlabel("Frames")
plt.ylabel("Coefficents")

plt.matshow(mfcc_feat2.T, cmap="Greys_r")
plt.xlabel("Frames")
plt.ylabel("Coefficents")

plt.matshow(mfcc_feat3.T, cmap="Greys_r")
plt.xlabel("Frames")
plt.ylabel("Coefficents")

plt.matshow(mfcc_feat4.T, cmap="Greys_r")
plt.xlabel("Frames")
plt.ylabel("Coefficents")

distance1, path1 = fastdtw(mfcc_feat1, mfcc_feat4, dist=euclidean)
distance2, path2 = fastdtw(mfcc_feat1, mfcc_feat2, dist=euclidean)
distance3, path3 = fastdtw(mfcc_feat1, mfcc_feat3, dist=euclidean)
distance4, path4 = fastdtw(mfcc_feat1, mfcc_feat1, dist=euclidean)


answers = [distance1, distance2, distance3, distance4]

rounded_ans = [round(x, 1) for x in answers]

print(rounded_ans)

plt.show()
