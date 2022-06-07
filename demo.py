import sndmath as s

#TODO: TOO LOUD
nechlsong = (
"1.0 e4  1.0 c4 1.0g4 1.0d4",
"1.0 f#4 1.0 e4 1.0b4 1.0f#4",
"1.0 a4  1.0 g4 1.0d5 1.0a4",
"1.0 a5  1.0 g5 1.0d4 1.0a5"
)


AmongUsMelody = """
0.4C4
0.4D#4
0.4F4
0.4F#4
0.4F4
0.4D#4
0.4C4
0.8p
0.2A#3
0.2D4
0.4C4
0.8p
0.4G3
0.4C4
0.4p
0.4C4
0.4D#4
0.4F4
0.4F#4
0.4F4
0.4D#4
0.4F#4
0.8p
0.2F#4
0.2F4
0.2D#4
0.2F#4
0.2F4
0.2D#4
0.4C4

"""

MotherRussia = """
0.5C4
1.0F4
0.75C4
0.25D4
1.0 E4
0.48 A3
0.02 p
0.5 A3

"""
# These sound like high-hats
beat = "0.03c4 0.97p 0.03c4 0.97p 0.03c4 0.97p 0.03c4"
beat2 = "0.5p 0.03c4 0.97p 0.03c4 0.97p 0.03c4 0.97p 0.03c4"



def Main():
    s.create_melody(MotherRussia, "russia.wav", s.SAW)
    s.instrument_melody(AmongUsMelody, "amogus.wav", s.PIANO)
    s.create_melodies((beat, beat2), "noise.wav", s.NOISE)
    s.create_melodies(nechlsong, "nechl.wav", s.SIN);

if __name__ == '__main__':
    Main()
