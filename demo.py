import sndmath as s
import os
import subprocess
# To play the audio, the program SOX a.k.a play has to be installed. This is because the file contains no metadata that specifies sample Rate, encoding and alike


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
    s.create_melody(MotherRussia, "russia.raw", s.SAW)
    s.instrument_melody(AmongUsMelody, "amogus.raw", s.PIANO)
    s.create_melodies((beat, beat2), "noise.raw", s.NOISE)
    ret = 0
    try:
        if os.name == 'posix':
            ret = subprocess.check_output(["which","play"])
        else:
            ret = subprocess.check_output(["where","play"]);
        # Play directly
        if ret:
            for fname in ("russia.raw", "amogus.raw", "noise.raw"):
                os.system(f"play --volume 2.0 --endian little -r 44100 -c 1 -e floating-point -b 32 {fname}")
    except Exception as e:
        print("Unable to play file data with SoX: ", e)

if __name__ == '__main__':
    Main()
