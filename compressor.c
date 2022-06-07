#include "soundwaves.h"
#include "debug.h"




// THIS ONE IS GARBAGE, TOO MUCH DISTORTION!! (GOOD DISTORTION PLUGIN :-) )
/* does not cut off/clip, introduces some distortion, cutoff must be >= 0!! */
void linearCompressor(floatArray_t signal, float cutoff, float intensity)
{
	int compcount = 0;
	for (int i = 0; i < signal.length; i++){
		if (signal.data[i] > cutoff){
			signal.data[i] = ((signal.data[i] - cutoff) *  intensity + (signal.data[i] - cutoff));
		} else if (signal.data[i] < 0.0f - cutoff ){ // Negative cutoff
			signal.data[i] = ((signal.data[i] + cutoff) *  intensity + (signal.data[i] - cutoff));
		}
	}

	DEBUG("%d samples compressed", compcount);
}

// TODO
/* stops at max, radius = max-cutoff */
void softCompressor(floatArray_t signal, float cutoff, float max);
