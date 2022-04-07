#ifndef __SOUNDWAVES_H
#define __SOUNDWAVES_H
// the C programm should not be able to change the SAMPLE_SIZE, this is done in Python,
// therefore CONST
// To avoid overoptimization, it is declared as VOLATILE because it might be changed in Python
extern const unsigned long SAMPLE_SIZE; // 44.1k is the default sample size

// Constants
#define OVERTONE_MAX 16

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

floatArray_t sinewave(float duration, float note);
floatArray_t sawtooth(float duration, float note);
floatArray_t squarewave(float duration, float note);
floatArray_t squaredSeries(float duration, float note);
floatArray_t overtone(float duration, float note, overtone_t * desc, wavefunc_t wavefunc);
floatArray_t joinwaves(floatArray_t arrays[], unsigned long size);


// __SOUNDWAVES_H
#endif
