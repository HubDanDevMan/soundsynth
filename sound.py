import ctypes as ct
import struct as st
from math import sin, pi
# from hashlib import sha256

SAMPLE_SIZE = 44100 # in hz
TONE_LENGTH = 1     # Duration in seconds
VOLUME = 1          # Loudness


CODE = "0.4C4,0.4D#4,0.4F4,0.4F#4,0.4F4,0.4D#4,0.4C4,0.8Ø,0.2A#3,0.2D4,0.4C4,0.8Ø,0.4G3,0.4C4,0.4Ø,0.4C4,0.4D#4,0.4F4,0.4F#4,0.4F4,0.4D#4,0.4F#4,0.8Ø,0.2F#4,0.2F4,0.2D#4,0.2F#4,0.2F4,0.2D#4,0.4C4"


def freqAt(index):
    return 2**(index/12)*440

NOTES = {
    "F#": freqAt(-15),
    "G3": freqAt(-14),
    "G#3": freqAt(-13),
    "A3": freqAt(-12),
    "A#3": freqAt(-11),
    "B3": freqAt(-10),
    "C4": freqAt(-9),
    "C#4": freqAt(-8),
    "D4": freqAt(-7),
    "D#4": freqAt(-6),
    "E4": freqAt(-5),
    "F4": freqAt(-4),
    "F#4": freqAt(-3),
    "G4": freqAt(-2),
    "G#4": freqAt(-1),
    "A4": freqAt(0),
    "A#4": freqAt(1),
    "B4": freqAt(2),
    "C5": freqAt(3),
    "C#5": freqAt(4),
    "D5": freqAt(5),
    "D#5": freqAt(6),
    "Ø": 0.1
}


def multilayer(*notes): # add float
    """Adds all the note channels (float arrays) to a single channel"""
    base = notes[0]
    print("Base:",len(base))
    for note in notes[1:]:
        print(base)
        for n, value in enumerate(note):
            if len(base) <= n: 
                base.append(value)
                print("added directly")
            else:
                base[n] += value
                print("appended: ",n)
    return base


def parseCode(c):
    """generates a single channel as a float array"""
    sound = []
    a = c.split(",")
    for note in a:
        sound += generateSound(NOTES[note[3:]], float(note[0:3]))
        c = c[2:]
    return sound

def soundAt(sampleIndex,tone):
    freq = SAMPLE_SIZE / tone
    return sin(sampleIndex / (freq / (pi*2)))

def generateSound(tone, length):
    """Returns a list of floats"""
    arr = []
    for index in range(int(SAMPLE_SIZE * length)):# TIME?
        arr += [
                soundAt(index, tone) * VOLUME ]
    return arr

def Main():
    res = parseCode(CODE) # Float array
    print("samples: ", len(res))
    with open("soundfile.raw", "wb") as f:
        f.write(b"".join([ct.c_float(s) for s in res])) # convert to c_float and join

if __name__ == '__main__':
     Main()
"""
    a = parseCode("1.3C4,0.7A4")
    b = parseCode("0.4F4,2.6C#4")
    sound = multilayer(a,b)
    if sound == b:
        print("equal")
    with open("soundfile.raw", "wb") as f:
        f.write(b"".join([ct.c_float(s) for s in sound]))
    """
