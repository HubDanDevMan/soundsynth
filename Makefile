PY=python3.8
PYBUILD=setup.py
PYINSTALL=install



default: setup.py
	$(PY) $(PYBUILD) build

install: default
	$(PY) $(PYBUILD) $(PYINSTALL)

