from distutils.core import setup, Extension

module = Extension("sndmath", sources=["sndmath.c", "melodyparser.c", "soundwaves.c", "harmonics.c", "clip.c", "wav.c", "multimel.c", "compressor.c"])

setup(name="sndmath", version="0.01",
        description="Create melodies in Python with a blazingly fast C extension", ext_modules=[module])

