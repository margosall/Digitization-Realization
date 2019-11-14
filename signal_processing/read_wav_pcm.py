import wave
import struct
import librosa

def read_whole(filename):
    wav_r = wave.open(filename, 'r')
    print(wav_r.getframerate())
    ret = []
    while wav_r.tell() < wav_r.getnframes():
        decoded = struct.unpack("<h", wav_r.readframes(1))
        ret.append(decoded[0])
    print(len(ret))
    return ret

print(read_whole("blender_downsampled.wav"))