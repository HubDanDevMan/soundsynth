#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "soundwaves.h"
#include "debug.h"
#include "clip.h"
#define IEEE_FLOAT_FORMAT 0x03



/* Data gathered by reverse engineering wav headers */
char wavdata[] = {
0x52, 0x49, 0x46, 0x46, 0x46, 0x00, 0x00, 0x00,
0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,  
0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
0x44, 0xac, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00, 
0x02, 0x00, 0x10, 0x00, 0x4c, 0x49, 0x53, 0x54,
0x1a, 0x00, 0x00, 0x00, 0x49, 0x4e, 0x46, 0x4f, 
0x49, 0x53, 0x46, 0x54, 0x0e, 0x00, 0x00, 0x00,
0x4c, 0x61, 0x76, 0x66, 0x35, 0x39, 0x2e, 0x32, 
0x30, 0x2e, 0x31, 0x30, 0x31, 0x00, 0x64, 0x61,
0x74, 0x61, 0x02, 0x00, 0x00, 0x00 /* Remaining bytes are data */
};


// Format is according to http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
// other source: https://gist.github.com/Jon-Schneider/8b7c53d27a7a13346a643dac9c19d34f
// https://web.archive.org/web/20141213140451/https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
// The packed attribute tells the compiler not to add memory alignment padding between the struct members
// If the members aren't packed, the padding would displace and corrupt the WAV header members.
/*
typedef struct __attribute__((packed)) { 
	// RIFF header 
	uint8_t RIFF_ID[4];
	uint32_t wavdatasize;	<-- This Value is modified in writeAsWAV()
	uint8_t WAVE_ID[4];
	// Format header
	uint8_t FMT_ID[4];
	uint32_t FMTChunkSize;
	uint16_t audioFormat;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t wBitsPerSample;
	uint16_t cbSize;
	uint16_t wValidBitsPerSample;
	// Data header
	uint32_t dwChannelMask;
	uint8_t SubFormat[16];
	...
	... 		<-- Channel length is set to size
} WAVHeader_t;
*/

unsigned long writeAsWAV(floatArray_t flar, FILE * f)
{
	/*WAVHeader_t WAVHeader = {

		{'R', 'I', 'F', 'F'},
		datasize - 8, // File size - 8 (sizeof RIFF header)
		{'W', 'A', 'V', 'E'},
		// Format Header
		{'F', 'M', 'T', ' '},
	};
	return memcpy(dest, &WAVHeader, sizeof(WAVHeader_t));
	*/
	short * result = malloc(sizeof(short)*flar.length);
	clipAndConvertSamples(flar, result);
	unsigned long res;
	int samplecount = flar.length;


	int * dest = (int*) (&wavdata[0x04]);
	*dest += 2*samplecount;
	dest = (int *) (&wavdata[0x4a]);
	*dest = 2*samplecount;

	res = fwrite(wavdata, sizeof(wavdata), 1, f);
	res += fwrite(result, sizeof(float), flar.length, f);
	if (res == sizeof(wavdata) + sizeof(short)*flar.length){
		DEBUG("REsults are matching");
	}
	return res;


}
