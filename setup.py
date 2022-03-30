from distutils.core import setup, Extension

module = Extension("sndmath", sources=["sndmath.c"])

setup(name="sndmath", version="0.01",
        description="speed opts for floats", ext_modules=[module])

