PY=python3
SCRIPT=sound.py
SND=pacat
SNDARGS=--volume 16000 --format float32ne
SNDFILE=soundfile

default: $(SCRIPT)
	$(PY) $(SCRIPT)

run: default
	$(SND) $(SNDARGS) $(SNDFILE)
