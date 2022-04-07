#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "soundwaves.h"



static void printfFlar(floatArray_t f){
	printf(
		"Synthesized %d samples\n", f.length
	);
}

const unsigned long SAMPLE_SIZE = 44100; // 44.1k is the default sample size
const float VOLUME = 0.3f; // 44.1k is the default sample size
#define FREQ(tone) (SAMPLE_SIZE / tone)


static inline float sineAt(unsigned long sample_index, float factor){
	return sin(sample_index/factor) * VOLUME;
}

static inline float squareAt(unsigned long sample_index, float factor){
	float x = sample_index/factor;
	return 	(sin(x) +
		(1.0f/3.0f) * sin(3.0f*x) +
		(1.0f/5.0f) * sin(5.0f*x) +
		(1.0f/7.0f) * sin(7.0f*x) +
		(1.0f/9.0f) * sin(9.0f*x) +
		(1.0f/11.0f) * sin(11.0f*x)) * VOLUME;
}

static inline float sawtoothAt(unsigned long sample_index, float factor){
	float x = sample_index/factor;
	return 	(sin(x) +
		(1.0f/2.0f) * sin(2.0f*x) +
		(1.0f/3.0f) * sin(3.0f*x) +
		(1.0f/4.0f) * sin(4.0f*x) +
		(1.0f/5.0f) * sin(5.0f*x) +
		(1.0f/6.0f) * sin(6.0f*x)) * VOLUME;
}

static inline float squaredseriesAt(unsigned long sample_index, float factor){
	float x = sample_index/factor;
	return 	(sin(x) +
		(1.0f/4.0f) * sin(4.0f*x) +
		(1.0f/9.0f) * sin(9.0f*x) +
		(1.0f/16.0f) * sin(16.0f*x) +
		(1.0f/25.0f) * sin(25.0f*x) +
		(1.0f/36.0f) * sin(36.0f*x))* VOLUME;
}


/* Basic sinewave digital oscillator */

floatArray_t sinewave(float duration, float note){
	float factor = (FREQ(note) / (2 * M_PI));
	floatArray_t flar;
	flar.length = duration * SAMPLE_SIZE; // time in seconds * (samples/second)

	// using malloc instead of PyMem_RawMalloc is fine for performance critical use cases such as this one :^)
	flar.data = malloc(sizeof(float) * flar.length);
	if (flar.data == NULL){
		flar.length = SND_MEMORY_ERR;
		return flar;
	}

	
	for (int sampleIndex = 0; sampleIndex < flar.length; sampleIndex++){
		flar.data[sampleIndex] = sineAt(sampleIndex, factor);
	}
	printfFlar(flar);
	
	return flar;
}


floatArray_t sawtooth(float duration, float note){
	float factor = (FREQ(note) / (2 * M_PI));
	floatArray_t flar;
	flar.length = duration * SAMPLE_SIZE; // time in seconds * (samples/second)

	// using malloc instead of PyMem_RawMalloc is fine for performance critical use cases such as this one :^)
	flar.data = malloc(sizeof(float) * flar.length);
	if (flar.data == NULL){
		flar.length = SND_MEMORY_ERR;
		return flar;
	}

	
	for (int sampleIndex = 0; sampleIndex < flar.length; sampleIndex++){
		flar.data[sampleIndex] = sawtoothAt(sampleIndex, factor);
	}
	
	printfFlar(flar);
	return flar;


}

floatArray_t squarewave(float duration, float note){
	float factor = (FREQ(note) / (2 * M_PI));
	floatArray_t flar;
	flar.length = duration * SAMPLE_SIZE; // time in seconds * (samples/second)

	// using malloc instead of PyMem_RawMalloc is fine for performance critical use cases such as this one :^)
	flar.data = malloc(sizeof(float) * flar.length);
	if (flar.data == NULL){
		flar.length = SND_MEMORY_ERR;
		return flar;
	}

	
	for (int sampleIndex = 0; sampleIndex < flar.length; sampleIndex++){
		flar.data[sampleIndex] = squareAt(sampleIndex, factor);
	}
	
	printfFlar(flar);
	return flar;


}

/* This waveform is comprised of numbers resulting squared */
floatArray_t squaredSeries(float duration, float note){
	float factor = (FREQ(note) / (2 * M_PI));
	floatArray_t flar;
	flar.length = duration * SAMPLE_SIZE; // time in seconds * (samples/second)

	// using malloc instead of PyMem_RawMalloc is fine for performance critical use cases such as this one :^)
	flar.data = malloc(sizeof(float) * flar.length);
	if (flar.data == NULL){
		flar.length = SND_MEMORY_ERR;
		return flar;
	}

	
	for (int sampleIndex = 0; sampleIndex < flar.length; sampleIndex++){
		flar.data[sampleIndex] = squaredseriesAt(sampleIndex, factor);
	}
	
	return flar;

}

/* This function generates a float array of a note with harmonic overtones */
floatArray_t overtone(float duration, float note, overtone_t *desc, wavefunc_t wavefunc){
	floatArray_t separate[OVERTONE_MAX];
	int overtoneCounter = 0;
	float oldNote = note;

	// Check if overtone descriptor is valid
	if (desc == NULL){
		separate[0].length = SND_INV_DESC_ERR;	// Set Errorflag
		return separate[0];
	}


	printf("First point\n");
	// Create and keep track of all separate overtones
	for (unsigned long overtoneIndex = 0; overtoneIndex < OVERTONE_MAX; overtoneIndex++){
		if (desc->values[overtoneIndex] != 0.0f) { // check wether harmonic is present or not  
			printf("In overtone, %d\n", overtoneCounter);
			separate[overtoneCounter] = wavefunc(duration, note); // Check for error

			printf("Called Wavefunc\n");
			// Check for invalid float arrays
			if (separate[overtoneCounter].length < 0){
				printf("neg len\n");
				while (overtoneCounter){		
					free(separate[overtoneCounter].data); // free all the previous
				}
				// set errorflag and return
				printf("Alloc fail\n");
				separate[0].length = SND_MEMORY_ERR;
				return separate[0];

			}
			overtoneCounter++;
		}
		note += oldNote;
	}


	printf("cleaning\n");
	// Add them all together and return resulting floatArray (first one in array separate)  
	for (unsigned long counter = 1; counter < overtoneCounter; counter++){
		for (unsigned long floatIndex = 0; floatIndex < separate[counter].length; floatIndex++){
			printf("adding\n");
			separate[0].data[floatIndex] += separate[counter].data[floatIndex];
		}
		// Free all the previously allocated floatArrays (but not the first one which is returned)
		// freeing NULL is fine according to C99, may happen if for loop ^^^ never runs (length < 0)
		free(separate[counter].data);
		printf("freed\n");
	}
	printf("leaving normally\n");
	return separate[0];
}


// Add all together and frees them
floatArray_t joinwaves(floatArray_t arrays[], unsigned long count){
	// find longest floatarray and use it as the base.
	// This float array will then be returnes
	unsigned long longest;
	unsigned long longestSize = 0;
	for (unsigned long i = 0; i < count; i++){
		if ( arrays[i].length > longestSize){
			longest = i;
			longestSize = arrays[i].length;
		}
	}
	// Add them together
	for (i = 0; i < count; i++){
		if (i != longest){
			for (unsigned long sample; sample < arrays[i].length; sample++){
				arrays[longest].data[sample] += arrays[i].data[sample]
			}
			// free unused array
			free(arrays[i].data);
		}
	}
	return arrays[longest];
}
