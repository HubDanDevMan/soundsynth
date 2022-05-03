## Huber, Daniel
## G4b


# Soundsynthesizer

### Overview

The result is a Python module called sndmath that can be imported and used to generate tones with the correct pitch and duration.

### Building and running

Type `make install` to build the C files. It runs the `setup.py` script and builds the python module. Compiling might only work with `gcc` due to 
compiler specific macros. The module can be used by importing sndmath in python (make sure correct version number is used for compilation and interpreter
invocation) and calling one of the following functions:

- `create_melody(melodystr, filename, osc)`
- `create_melodies(melodystr_tuple, filename, osc)`
- `create_instrument(melodystr, filename, instrument_enum)`

The melody string must have a format as described below:
```
syntax = note, {WHITESPACE}, {note};
note	= DURATION, {WHITESPACE}, (TONE, OCTAVENUMBER) | PAUSE;
duration= float;
TONE	= KEY;
PAUSE	= "p" | "P"
KEY	= ("a" | "b" | "c" | "d" | "e" | "f" | "g" | "A" | "B" | "C" | "D" | "E" | "F" | "G"), ["#"];
WHITESPACE = " " | "\n" | "\t"
NOTE: There is no E# and no B#
```
Example melody code:
`0.5 f#4 0.5C3	1.0D3`

`create_melodies()` takes a tuple of melodystrings as the first argument, the remaining ones are identical to `create_melody()`. The second argument
passed to the functions is the name of the output file, e.g. 'output.raw'. The output file will contain raw little-endian float PCM values ranging
from -1.0 to 1.0. It can be played by using the `play --volume 2.0 --endian little -r 44100 -c 1 -e floating-point -b 32 <OUTPUT_FILE>` command wich
is automatically invoked in the `demo.py` script (if SoX a.k.a play is available on the system). The third argument for the first two functions `osc`
is an integer enum and is intended to be one of the following:

- sndmath.SIN
- sndmath.SAW
- sndmath.SQUARE
- sndmath.SQUAREDSERIES
- sndmath.NOISE

The `create_instrument()` functions last argument is an instrument enum but only `sndmath.PIANO`is valid as of now.
All functions are demonstrated when running the `demo.py` file and if `SoX` is is installed, the resulting melodies will be played automatically.
I can't stress enough that the version of Python interpreter invoking `setup.py` must be identical to the one that imports the module!

### Structure

High level code overview:

- `sndmath.c` contains all functions that can be called from Python. It contains the module initializer function that sets up everything. This file fulfills the main target goal, glueing everything together as a Python module.
- `melodyparser.c` contains functions that parse the melody strings. `parseNext()` returns the frequency and duration of the next note in a melody string. There are also functions such as `lookup()` to translate note strings to frequencies and `verifyNote()` that makes sure the melody string contains valid characters. `freqAt()` calculates the correct frequency of a note.  This file accomplishes the next target goal.
- `harmonics.c` contains functions for generation instrumental soundwaves such as `pianoNote()` and `pianoStroke()` as well as harmonic overtones related functions like `applyOvertones()` and `dynamicHarmonics()`. The latter two make the digital piano timbre sound a bit more natural.
- `soundwaves.c` contains oscillator functions that generate basic waves such as sine, sawtooth and square waves according to a frequency. The relevant functions to create the aforementioned waves are `sineAt()`, `sawtoothAt()` and `squareAt()`. I also added some peculiar "wave" functions such as `noiseAt()` and `squaredseriesAt()`. The noise function sounds like TV-static and the squaredseriesAt is my personal invention; it sounds a bit like a bell. This file also contains the `createNote()` function that creates a float array containing PCM values of an oscillator according to a frequency and duration. This file accomplishes the last target goal.
- `debug.h` contains a debug macro called `DEBUG(fmt, ...)` which is like libC's `printf()` except it provides more debugging relevant information.
- `*.h` contain header relevant data, ifndef guards and function prototypes.
- Scattered amongst most files are initializer functions for the lookup tables for oscillators, instruments and note frequencies. These are called from the Python module initializer function in `sndmath.c`

### Reflexion

I feel that reaching my target goals was not that difficult. The melodyparser was not that difficult to implement. Althoug, I am taking a bit of a short cut by ~ignoring a bug~ creating a feature 😄. If the melodystring specifies a duration followed immediatly by an 'E', the `atof()` function parses the letter 'E' as an exponent, raising the duration to the octave number that follows the 'E'. Instead of checking for said case, the program just returns an error.
Therefore, I recommend that every duration and tone in the melody string is separated by a whitespace character. The music theory aspect, especially
creating the correct wave length and amplitude was also trivial to me, because I recently had the topic oscillations in physics. I decided to extend
my program with functions that approach the sound of a piano, this was the second most difficult part of my project. Sounds generated by non-digital
instruments are very complex and unique. These instruments contain harmonic overtones, i.e. they resonate on the base frequency as well as some positive
integer multiples of the base frequency. I use maximum 16 harmonic overtones in my program. The amplitude of the individual harmonic overtones varies from
instrument to instrument and I tried to approach the sound of a piano using a spectrogram analyzer referenced in the sources. Compared with
a real piano, my program's result analyzed with the spectrogram looks quite similar to the real pianos spectrogram. 
Because a piano produces a tone by hitting a string with a hammer, the tone
contains a clicky sound in the beginning. I produced this click with the function `pianoStroke()`. I didn't really have to debug the aforementioned function but
I had to adjust the parameters of the waves that generate the clicky sound. Clicky sound is produced by inharmonic overtones of the base frequency.
This means the overtones are non-integer multiples of the base frequency. The overtones also change over time when the note is played. This change
of overtone amplitude is done in the function `dynamicOvertones()` and it makes the sound feel much more natural. An Instruments loudness also changes
over time and in my program I accomplished this using envelopes. Envelopes describe how the sound loudness changes over time. A good source on envelopes
and how they work is on Wikipedia, the link is listed in the sources. The envelope is applied in the function `pianoNote()`, which generates a piano note.
I have put in a lot of effort into approaching the piano sound, but it wasn't actually not a target goal and I did it because I got slightly carried away. Creating the piano was by far my favourite part of this project. But looking back I spend way too much time on it, even though it wasn't really part of the target goals. I did mention 'functions from music theory' in the target goal proposal list and I will count it as such.
The last target goal was writing the Python interface part of my program. All functions and enum constants that will be used inside the Python module are defined in the file `sndmath.c`. I needed a primer to understand Pythons inner working and a tutorial by RealPython really helped me.
I haven't managed to write a `create_instruments()` function, I don't have enough time for that yet. But the core target goal of writing a Python module
was reached. I often encountered errors when allocating memory because I malloc'd only the count of floats instead of the count of floats times the size
of a float. The debugging process of a Python C extension is completely different to debugging regular C programs. Especially
memory debugging errors are a lot harder to debug. I tried using `valgrind` but I couldn't really get it to understand what went wrong. Because Python
uses reference counting, I had to learn how to use refcount. Memory handling with malloc() and free() can be cumbersome to debug, but with `Py_DECREF()`
and `Py_INCREF()` I was struggling a bit. Refcounting is a new paradigm to me and a few times I suspected that there were some errors in my C extension. A few days ago, I ran a demo
program a few hundred times and the memory consumption increased by a lot. Now I managed to find the errors and I am confident that there are no leaks now.
I think that languages such as Rust are really beneficial to counter memory leaks, but because I already used C, I had to improvise a bit. Debugging was made
possible by creating and using the `DEBUG` macro which can be toggle on and off in the `debug.h` file. Parsing the debug logs can be a bit difficult,
therefore I added automatic calling file and line numbering to every debug log line.
Overall, I really liked that I wrote a Python C extension, I learned a lot in this process. I also got a better understanding of music synthesis theory. But I
had difficulties debugging the C extension while it was running, which is why I will try to improve my knowledge of memory debuggers such as valgrind.


### Sources:
- [C API Introduction Tutorial by RealPython](https://realpython.com/build-python-c-extension-module/)
- [C Standart Library Documentation](https://www.tutorialspoint.com/c_standard_library/index.htm)
- [Python C API](https://docs.python.org/3/c-api/index.html)
- [Wikipedia Synthesizer](https://en.wikipedia.org/wiki/Synthesizer)
- [Frequency Formula](https://www.translatorscafe.com/unit-converter/en-US/calculator/note-frequency/)
- [Envelopes](https://en.wikipedia.org/wiki/Envelope_(music))
- [Piano Harmonics](https://spectrogram.sciencemusic.org/)
- [GCC Macro Manual](https://gcc.gnu.org/onlinedocs/cpp/Macros.html)
