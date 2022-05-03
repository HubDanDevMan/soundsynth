#ifndef __SOUNDWAVES_H
#define __SOUNDWAVES_H
//#include "harmonics.h"
// the C programm should not be able to change the SAMPLE_SIZE, this is done in Python,
// therefore CONST
// To avoid overoptimization, it is declared as VOLATILE because it might be changed in Python
extern const unsigned long SAMPLE_SIZE; // 44.1k is the default sample size
extern const float VOLUME;


#define OSC_COUNT 5
typedef enum {
	SIN,
	SAW,
	SQUARE,
	SQUAREDSERIES,
	NOISE
} OSC;

// Errors
/* If malloc fails */
#define SND_MEMORY_ERR (-1)
/* If harmonic overtone descriptor is invalid */
#define SND_INV_DESC_ERR (-2)


// Assure free after use to avoid memory leaks
typedef struct {
	float * data;		// malloced float array
	signed int length;	// length in sample count or error if negative value
} floatArray_t;



// Make function ptrs readable
typedef floatArray_t (*wavefunc_t)(float,float); 
typedef float (*osc_t)(unsigned long,float); 

//floatArray_t sinewave(float duration, float note);
floatArray_t createNote(float duration, float note, OSC osc);
//floatArray_t overtone(float duration, float note, overtone_t * desc, wavefunc_t wavefunc);
floatArray_t joinwaves(floatArray_t arrays[], unsigned long size);
void initOscillators(void);

float sineAt(unsigned long sample_index, float factor);
float squareAt(unsigned long sample_index, float factor);
float squaredseriesAt(unsigned long sample_index, float factor);
float noiseAt(unsigned long sample_index, float factor);
float sawtoothAt(unsigned long sample_index, float factor);
float FREQ(float tone);


extern osc_t oscillators[OSC_COUNT];
// __SOUNDWAVES_H
#endif
