PY=python3
SCRIPT=sound.py
SND=pacat
SNDARGS=--volume 16000 --format float32ne
SNDFILE=soundfile.raw
OS=$(shell uname)
DUMMY=0


ifeq ($(OS),Darwin)
	SND=play
	SNDARGS=--volume 2.0 --endian little -r 44100 -c 1 -e floating-point -b 32
endif

default: $(SCRIPT)
	@echo PLAYING ON $(OS) with $(SND)
	$(PY) $(SCRIPT)

run: default 
	@echo Player is $(SND) with args $(SNDARGS)
	$(SND) $(SNDARGS) $(SNDFILE)
