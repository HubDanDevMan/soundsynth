#include <stdio.h>
#include "soundwaves.h"
#include <math.h>



int main()
{
	overtone_t o = {"Piano", {1.0, 0.8, 0.6, 0.2, 0.4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};
	floatArray_t result0 = sinewave(1.0, 440);
	floatArray_t result1 = squarewave(1.0, 440);
	floatArray_t result2 = sawtooth(1.0, 440);
	floatArray_t result3 = overtone(1.0, 440, &o, &sinewave);
	printf("%d\n", result3.length);
	FILE * myFile = fopen("coutput.raw", "wb");
	fwrite(result0.data, result0.length, sizeof(float), myFile );
	fwrite(result1.data, result1.length, sizeof(float), myFile );
	fwrite(result2.data, result2.length, sizeof(float), myFile );
	fwrite(result3.data, result3.length, sizeof(float), myFile );


	return 0;




}
