#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "soundwaves.h"
#include "harmonics.h"
#include "melodyparser.h"
osc_t oscillators[OSC_COUNT];

const unsigned long SAMPLE_SIZE = 44100;	// 44.1k is the default sample size
float VOLUME = 0.4f; 			// Default volume, less than 1 to ensure less clipping is performed at the end


float FREQ(float tone){
	return ((float) SAMPLE_SIZE / tone);
}


float sineAt(unsigned long sample_index, float factor){
	return sin(sample_index/factor) * VOLUME;
}

float squareAt(unsigned long sample_index, float factor){
	float x = sample_index/factor;
	return 	(sin(x) +
		(1.0f/3.0f) * sin(3.0f*x) +
		(1.0f/5.0f) * sin(5.0f*x) +
		(1.0f/7.0f) * sin(7.0f*x) +
		(1.0f/9.0f) * sin(9.0f*x) +
		(1.0f/11.0f) * sin(11.0f*x)) * VOLUME;
}

float sawtoothAt(unsigned long sample_index, float factor){
	float x = sample_index/factor;
	return 	(sin(x) +
		(1.0f/2.0f) * sin(2.0f*x) +
		(1.0f/3.0f) * sin(3.0f*x) +
		(1.0f/4.0f) * sin(4.0f*x) +
		(1.0f/5.0f) * sin(5.0f*x) +
		(1.0f/6.0f) * sin(6.0f*x)) * VOLUME * 0.7;	// Reduced amplitude due to Sawtooth wave being louder
}

float squaredseriesAt(unsigned long sample_index, float factor){
	float x = sample_index/factor;
	return 	(sin(x) +
		(1.0f/4.0f) * sin(4.0f*x) +
		(1.0f/9.0f) * sin(9.0f*x) +
		(1.0f/16.0f) * sin(16.0f*x) +
		(1.0f/25.0f) * sin(25.0f*x) +
		(1.0f/36.0f) * sin(36.0f*x))* VOLUME;
}

float noiseAt(unsigned long sample_index, float factor){
	return sin(rand()) * VOLUME*0.6;		// returns random float in codomain {-1.0 - 1.0}
}

/* Basic digital oscillator */

floatArray_t createNote(float duration, float note, OSC osc){
	DEBUG("Creating Note with duration %f and frequency %f", duration, note);
	float factor = (FREQ(note) / (2.0f * M_PI));
	floatArray_t flar;
	flar.length = duration * SAMPLE_SIZE; // time in seconds * (samples/second)

	if (note == -1.0f){
		DEBUG("Frequency is -1.0f, i.e. DONE PARSING");
		flar.length = 0;
		return flar;
	} 
	// using malloc instead of PyMem_RawMalloc is fine for performance critical use cases such as this one :^)
	DEBUG("Allocating %lu bytes", sizeof(float) * flar.length);
	flar.data = malloc(sizeof(float) * flar.length);
	if (flar.data == NULL){
		flar.length = SND_MEMORY_ERR;
		DEBUG("Memory allocation failed!");
		return flar;
	}
	if (note == PAUSE_FREQ){	// Check if note is a pause and optimize 
		// using memset to set silent pcm values with memset. This speeds up the synthesis significantly.
		DEBUG("Optimizing pause with duration %f by setting %lu bytes to 0, %d floats", duration, flar.length*sizeof(float), flar.length);
		memset(flar.data, 0x00, flar.length*sizeof(float));
		DEBUG("Synthesized %d samples", flar.length)
		return flar;
	}

	
	for (int sampleIndex = 0; sampleIndex < flar.length; sampleIndex++){
		flar.data[sampleIndex] = (*oscillators[osc])(sampleIndex, factor);
	}
	DEBUG("Synthesized %d samples", flar.length)
	return flar;
}



// Add all together and frees them
floatArray_t joinwaves(floatArray_t arrays[], unsigned long count){
	DEBUG("Joining %lu melodies...", count);
	if (count == 1){
		return arrays[0];
	}
	// find longest floatarray and use it as the base.
	// This float array will then be returned
	long longest = 0;
	long longestSize = 0;
	int sample;
	unsigned long i;
	for (i = 0; i < count; i++){
		if ( arrays[i].length > longestSize){
			longest = i;
			longestSize = arrays[i].length;
		}
	}
	DEBUG("Longest arrayindex is %lu and its length is %ld", longest, longestSize);
	// Add them together
	for (i = 0; i < count; i++){
		if (i != (unsigned long) longest){
			for (sample = 0; sample < arrays[i].length; sample++){
				arrays[longest].data[sample] += arrays[i].data[sample];
			}
			DEBUG("Added %d samples to longest array's data", sample);
			DEBUG("Freeing %d floats at location %p", arrays[i].length, arrays[i].data);
			free(arrays[i].data);			// free unused array
		}
	}
	return arrays[longest];
}

void initOscillators(void){
	oscillators[0] = &sineAt;
	oscillators[1] = &sawtoothAt;
	oscillators[2] = &squareAt;
	oscillators[3] = &squaredseriesAt;
	oscillators[4] = &noiseAt;
}
