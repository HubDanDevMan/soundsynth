#ifndef _HARMONICS_H
#define _HARMONICS_H

#include "soundwaves.h"

#define OVERTONES_MAX 16


typedef enum {
	PIANO,
	INSTRUMENT_END
} Instrument;

typedef struct {
	float attack;			// duration in seconds
	float attacklevel;		// intensity, usually around 1.25
	float decay;			// duration in seconds
	float sustainlevel;		// intensity, usually around 0.8
	float release;			// duration in seconds
} env_t;

typedef struct {
	env_t env;
	float overtones[OVERTONES_MAX];
} instrument_t;


extern floatArray_t(*INSTRUMENTS[INSTRUMENT_END])(float, float);
extern instrument_t instrdata[INSTRUMENT_END];


void initInstruments(void);
floatArray_t pianoNote(float duration, float freq);


/* _HARMONICS_H */
#endif
