import ctypes as ct
import struct as st
from math import sin, pi
from lark import Lark, Transformer

SAMPLE_SIZE = 44100 # in hz
TONE_LENGTH = 1     # Duration in seconds
VOLUME = 0.3          # Loudness

grammar = """

    ?start : note ("," note)*
    ?note : zahl value
    !value : (/[a-hA-H]/ ("#")? /[3-5]/)
        | "Ø"

    !zahl : NUMBER

    %import common.NUMBER
    %import common.WS
    %ignore WS
"""
CODE = "0.4C4,0.4D#4,0.4F4,0.4F#4,0.4F4,0.4D#4,0.4C4,0.8Ø,0.2A#3,0.2D4,0.4C4,0.8Ø,0.4G3,0.4C4,0.4Ø,0.4C4,0.4D#4,0.4F4,0.4F#4,0.4F4,0.4D#4,0.4F#4,0.8Ø,0.2F#4,0.2F4,0.2D#4,0.2F#4,0.2F4,0.2D#4,0.4C4"

class NotenTransformer(Transformer):
    NUMBER = float
    notes = []
    def note(self, items):
        print(f"playing {items[1]} for {float(items[0])} seconds")
        #notes.append(self.larkInterpreteCode(items[1],items[0]))
    
    def larkInterpreteCode(self, note, duration):
        return generateSound(note, duration)


    def value(self,items):
        return "".join(items)
    
    
def freqAt(index):
    return 2**(index/12)*440

NOTES = {
    "F#3": freqAt(-15),
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
    for note in notes[1:]:
        for n, value in enumerate(note):
            if len(base) <= n: 
                base.append(value)
            else:
                base[n] += value
    return base


def parseCode(c,func):
    """generates a single channel as a float array"""
    sound = []
    a = c.split(",")
    for note in a:
        sound += generateSound(NOTES[note[3:]], float(note[0:3]), func)
        c = c[2:]
    return sound

def soundAt(sampleIndex,tone):
    freq = SAMPLE_SIZE / tone
    return sin(sampleIndex / (freq / (pi*2)))

def chirp(start, end, length, func): # LENGHT in seconds
    # linear interpolation
    signal = []
    steps = length*256 # calculate if step width is long
    incrementTime = length / steps
    incrementFreq = (end - start) / steps
    for _ in range(steps):
        signal += generateSound(start, incrementTime,func) # tone, length, function
        start += incrementFreq
    print("len", len(signal))
    return signal

def soundAtSquare(sampleIndex, tone):
    freq = SAMPLE_SIZE / tone
    x = sampleIndex/(freq/(pi*2))
    return (sin(x) +
        (1/3)*sin(3*x) +
        (1/5)*sin(5*x) +
        (1/7)*sin(7*x) +
        (1/9)*sin(9*x) +
        (1/11)*sin(11*x) +
        (1/13)*sin(14*x))


def soundAtSawtooth(sampleIndex, tone):
    freq = SAMPLE_SIZE / tone
    x = sampleIndex/(freq/(pi*2))
    return (sin(x)+
            (1/2)*sin(2*x)+
            (1/3)*sin(3*x)+
            (1/4)*sin(4*x)+
            (1/5)*sin(5*x)+
            (1/6)*sin(6*x)+
            (1/7)*sin(7*x))


def generateSound(tone, length, func):
    """Returns a list of floats"""
    arr = []
    for index in range(int(SAMPLE_SIZE * length)):# TIME?
        arr += [
                overtone(index, tone, func) * VOLUME ] # ADD HARMONICs
    # clip end of note for smooth transition
    if len(arr) > 1000:
        smoothSampleCount = 1000
    else:
        smoothSampleCount = int(len(arr)/3)
    smoothRate = 0.9995
    base = len(arr)-smoothSampleCount
    for offset in range(smoothSampleCount):
        arr[base+offset] = arr[base+offset] * smoothRate
        smoothRate = smoothRate**smoothSampleCount
        print("rate:",smoothRate,"smoothSampleCount:", smoothSampleCount)
    return arr

def overtone(sampleIndex, basefreq, func=soundAt):
    return func(sampleIndex, basefreq) + 0.6*func(sampleIndex, basefreq*2) + 0.5*func(sampleIndex, basefreq*4) + 0.25*func(sampleIndex, basefreq*3)

def Main():
    res = parseCode(CODE, soundAtSawtooth) # Float array
    res2= parseCode(CODE, soundAt) # Float array
    with open("soundfile.raw", "wb") as f:
        f.write(b"".join([ct.c_float(s) for s in (res + res2 + chirp(440,880,2,soundAtSquare) + generateSound(880,2,soundAtSquare))]))#multilayer(res, res2)])) # convert to c_float and join"""

if __name__ == '__main__':
    #parser = Lark(grammar)
    #tree = parser.parse(CODE)
    #NotenTransformer().transform(tree)
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
