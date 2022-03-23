import ctypes as ct
import struct as st
from math import sin, pi
# from hashlib import sha256

SAMPLE_SIZE = 44100 # in hz
TONE_LENGTH = 1     # Duration in seconds
VOLUME = 1          # Loudness


CODE = "0.4C4,0.4D#4,0.4F4,0.4F#4,0.4F4,0.4D#4,0.4C4,2.0Ø"


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
    "Ø": 34000
    }


def multilayer(*notes): # add float
    """Adds all the bote channels to a single channel"""
    base = bytearray(b"\x00")
    for n, note in enumerate(notes):
        i = 0
        while note:
            print(base[i:i+4][0], note[0:4][0][0],"\n*\n") 
            base += bytes(ct.c_float(
                         base[i:i+4][0] + note[0:4][0]
                    )
                )
            note = note[4:]
            i += 4
            print(len(note))
    return bytes(base)


def parseCode(c):
    sound = []
    a = c.split(",")
    for note in a:
        sound.append(generateSound(
                NOTES[note[3:]], float(note[0:3]))
                )
        c = c[2:]
    return sound

def soundAt(sampleIndex,tone):
    freq = SAMPLE_SIZE / tone
    return sin(sampleIndex / (freq / (pi*2)))

def generateSound(tone, length):
    arr = []
    for index in range(int(SAMPLE_SIZE * length)):# TIME?
        arr.append(
                soundAt(index, tone) * VOLUME)
    return arr

def Main():
    sound = parseCode(CODE)
    print(len(sound))
    with open("soundfile", "wb") as f:
        f.write(b"".join([ct.c_float(s) for s in sound]))

if __name__ == '__main__':
    Main()
    """
    a = parseCode("1.3C4,0.7A4")
    b = parseCode("0.4F4,1.6C#4")
    sound = multilayer(a,b)
    with open("soundfile", "wb") as f:
        f.write(sound)"""
