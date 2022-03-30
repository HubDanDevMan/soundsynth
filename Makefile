PY=python3
PYBUILD=setup.py
PYINSTALL=install
SCRIPT=sound.py
SND=pacat
SNDARGS=--volume 16000 --format float32ne
SNDFILE=soundfile.raw
OS=$(shell uname)


ifeq ($(OS),Darwin)
	SND=play
	SNDARGS=--volume 2.0 --endian little -r 44100 -c 1 -e floating-point -b 32
endif

default: setup.py
	$(PY) $(PYBUILD) 

install: default
	$(PY) $(PYBUILD) $(PYINSTALL)

run: default $(SCRIPT)
	$(PY) $(SCRIPT)
	@echo Player is $(SND) with args $(SNDARGS)
	@echo PLAYING ON $(OS) with $(SND)
	$(SND) $(SNDARGS) $(SNDFILE)
