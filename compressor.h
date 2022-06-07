#ifndef _COMPRESSOR_H
#define _COMPRESSOR_H

#include "soundwaves.h"



/* does not cut off/clip, introduces some distortion */
void linearCompressor(floatArray_t signal, float cutoff, float intensity);
/* stops at max, radius = max-cutoff */
void softCompressor(floatArray_t signal, float cutoff, float max);

/* _COMPRESSOR_H */
#endif
