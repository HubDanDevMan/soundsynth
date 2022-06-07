import timeit

SETUP = 'import sndmath'
TEST_CODE = 'sndmath.instrument_melody("0.5 d#4 0.5 d#5 0.5 d#4 0.5 d#5 0.5 d#4 0.5 d#5 0.5 d#4 0.5 d#5 0.5 d#4 0.5 d#5 0.5 d#4 0.5 d#5 0.5 d#4 0.5 d#5 0.5 d#4 0.5 d#5 0.5 d#4 0.5 d#5 0.5 d#4 0.5 d#5 0.5 d#4 0.5 d#5", "_perf.wav", sndmath.SIN)'

times = timeit.repeat(setup = SETUP,
                            stmt = TEST_CODE,
                            number = 100)

print(f"Took {times[0]} seconds")
