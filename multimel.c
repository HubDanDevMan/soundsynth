#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "debug.h"
#include "soundwaves.h"
#include "harmonics.h"


/* ignores releasephase of previous note */
floatArray_t joinNotes(floatArray_t result, floatArray_t note)
{
	if (result.data == NULL){
		result.data = note.data;
		result.length = note.length;
		return result;
	}
	result.data = realloc(result.data, sizeof(float)*(result.length + note.length));
	memcpy( &(result.data[result.length]), note.data, sizeof(float)*note.length);
	result.length = result.length + note.length;
	free(note.data);
	return result;
}


/* adds to previous release phase, instrument MUST be valid */
floatArray_t joinNotesInstrument(floatArray_t result, floatArray_t note, int instr){
	int releaseCount = instrdata[instr].env.release * SAMPLE_SIZE;

	if (result.data == NULL){ 	// not first note
		result.data = note.data;
		result.length = note.length;
		return result;
	}

	// Add to previous release signal
	for (int i = 0; i < releaseCount; i++){
		result.data[result.length + i - releaseCount] += note.data[i];
	}

	int reallocSizeBytes = (result.length + note.length - releaseCount) * sizeof(float);
	result.data = realloc(result.data, reallocSizeBytes);

	/*	previous note end		current note without prev rel phase	floatcount without previous release phase */
	memcpy( &(result.data[result.length]) , &(note.data[releaseCount]), sizeof(float)*(note.length - releaseCount));
	result.length = result.length + note.length - releaseCount;

	free(note.data);
	return result;
}
