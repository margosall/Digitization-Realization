import scipy.io.wavfile as wav
import librosa
import numpy as np
import matplotlib.pyplot as plt
from python_speech_features import mfcc



filenames = ["blender2_downsampled.wav" ,"gun2_downsampled.wav", "water_downsampled.wav", "humvee_downsampled.wav", "blender_downsampled.wav"]


def extract_features(filename):

    y, s = librosa.load(filename, sr=16000, mono=True)

    # int16_samples = []

    # for sample in y:
    #     new_value = np.int16((32767 - -32768)/(1 - -1)*(sample - 1) + 32767)
    #     int16_samples.append(new_value)


    features = mfcc(y, samplerate=16000, winlen=0.03, winstep=0.03, appendEnergy=1)
    # remove 0 index coeff
    features = np.delete(features, (0), axis=1)

    # mean = np.mean(features, axis=0)
    # std = np.std(features, axis=0)

    # features = (features - mean) / std

    c_string = "csf_float " + filename.split("_")[0] + "_mfcc[" + str(features.shape[0] * features.shape[1]) + "] = {"

    for frame in features:
        for feature in frame:
            c_string += str(feature) + ", "

    c_string += "};"

    # plt.matshow(features)

    print(c_string)

    plt.show()

for filename in filenames:
    extract_features(filename)