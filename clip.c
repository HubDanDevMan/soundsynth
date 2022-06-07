#include "debug.h"
#include "soundwaves.h"
#include <stdio.h>
#include <stdlib.h>



/* this function clips samples and converts the 32-bit float data to PCM signed 16-bit data */
/* s16le is more common than f32le and therefore more widely supported by audio players */
short* clipAndConvertSamples(floatArray_t data, short* dest)
{
	unsigned long clipCount = 0;

	for (int i = 0; i < data.length; i++){
		if (data.data[i] > 1.0f){
			data.data[i] = 1.0f;
			clipCount++;
		}
		else if (data.data[i] > 1.0f){
			data.data[i] = -1.0f;
			clipCount++;
		}
		dest[i] = (short) (data.data[i] * (1<<15));
	}
	// display percentage
	DEBUG("%lu of %d samples clipped [%f%%]\n", clipCount, data.length, clipCount ? ((float)clipCount/(float)data.length)*100.0f : 0.0); // if clipped samples count is too high, adjust domain and adjust volume
	return dest;
}
