#include "debug.h"
#include "harmonics.h"
#include "soundwaves.h"
#include "melodyparser.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


floatArray_t (*INSTRUMENTS[INSTRUMENT_END])(float, float);
instrument_t instrdata[INSTRUMENT_END] = {
	/* atk, lvl,  dec, sus, rel |   1.    2.     3.      4.   harmonic ... */
	/* Piano */
	{{ 0.15, 1.25, 0.8, 0.8, 0.4}, {1.0f, 0.15f, 0.068f, 0.043f, 0.042f, 0.025f, 0.028, 0.026f, 0.006f, 0.005}}


};

static float pianoStroke(int index)
{	const float CLICK_SAMPLE_DURATION = 3000.0f; 
	const float freq = 260.0f;
	float amplitude= ((float) index) / CLICK_SAMPLE_DURATION;
	float fac = FREQ(freq)/(2*M_PI);
	if (index > (int)CLICK_SAMPLE_DURATION){
		amplitude = 1.0f;
	}
	return (1.0f-amplitude) * 4.8f * (
			sineAt(index, fac*1.21) * 0.2 +
			sineAt(index, fac*1.14) * 0.2 +
			sineAt(index, fac*1.06) * 0.3 +
			sineAt(index, fac*0.93) * 0.5 +
			sineAt(index, fac*0.96) * 0.5 
	);
}



static float applyOvertone(float* overtones, int sampleIndex, float freq, int osc)
{
	float res = 0;
	int i;
	for (i = 0; i<OVERTONES_MAX; i++){
		if (overtones[i] != 0.0f){
			res += (*oscillators[osc])(sampleIndex, FREQ(freq)/(2.0f*M_PI)/(i+1)) * overtones[i];
		}
	}
	return res;
}



/* This function modifies the harmonic overtones of an instrument. The values of the harmonic intensities vaies during the whole note
 * In general, the upper harmonics get more silent as the tone is produced by the instrument.
 * */
static void dynamicHarmonics(float * harmonics)
{
	// Only works for piano currently
	float change[OVERTONES_MAX] = {0.0f, -0.00000004f, -0.00000012f, -0.00000016, -0.00000036, -0.00000025, -0.0000002, -0.0000002, -0.00000016, -0.00000024, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	#define DYN_HARMONIC_FACTOR 0.99996
	for (int i = 0; i < OVERTONES_MAX; i++){// modify the upper harmonics
		if (harmonics[i] > 0.0005f ){
			harmonics[i] += change[i];
		} else {
			harmonics[i] = 0.0f;
		}
	}

}

floatArray_t pianoNote(float duration, float frequency)
{
	DEBUG("Creating Piano note with duration %f and frequency %f", duration, frequency);
	float overtones[OVERTONES_MAX];					// Local copy of harmonics
	env_t env = instrdata[PIANO].env;				// Local copy of envelopw
	const int LEFTOVER = (int) (env.release * SAMPLE_SIZE);		// What remains of Release time in samples
	const int BASE_DURATION = duration * SAMPLE_SIZE;		// Sample count of base duration without RELEASE 
	const int TOTAL_DURATION = BASE_DURATION + LEFTOVER;		// Sample count of total note length 
	float fac;							// Factor of sine wave
	floatArray_t flar;						// Result
	int sigIndex = 0;						// current sample of signal being
	int releaseCount;
	float clickSample;
	
	flar.length = TOTAL_DURATION;
	flar.data = malloc(flar.length*sizeof(float));	// Allocate memory with enough space for note relese
	
	if (frequency == PAUSE_FREQ){
		// using memset to set silent pcm values with memset. This speeds up the synthesis significantly.
		DEBUG("Optimizing pause with duration %f by setting %lu bytes to 0, %d floats", duration, flar.length*sizeof(float), flar.length);
		memset(flar.data, 0x00, flar.length*sizeof(float));
		DEBUG("Synthesized %d samples", flar.length)
		return flar;
	}



	for (int i = 0; i < OVERTONES_MAX; i++){
		overtones[i] = instrdata[PIANO].overtones[i];
	}

	for (int i = 0; i < (int)(duration*SAMPLE_SIZE) + LEFTOVER; i++){
		fac = FREQ(frequency)/(2.0f*M_PI);
		clickSample = pianoStroke(i);
		flar.data[i] = VOLUME*(sineAt(i, fac*1.001)*0.4 + applyOvertone(overtones, i, frequency, SIN)) + clickSample;
		dynamicHarmonics(overtones);
	}
	DEBUG("Harmonics: %f %f %f %f %f %f %f %f", overtones[0], overtones[1], overtones[2], overtones[3], overtones[4], overtones[5], overtones[6], overtones[7]);
	for (int i = 0; i < 20; i++){
		DEBUG("click sample: %f", pianoStroke(i));
	}
	// Apply envelope
	DEBUG(	"Applying envelope with:\n"
		"\t\t\t\tattack:\t\t%f\n"
		"\t\t\t\tattacklevel:\t%f\n"
		"\t\t\t\tdecay:\t\t%f\n"
		"\t\t\t\tsustainlevel:\t%f\n"
		"\t\t\t\trelease:\t%f", env.attack, env.attacklevel,
		env.decay, env.sustainlevel, env.release);

	float factor = 0.0f;
	const int atkCount = env.attack * SAMPLE_SIZE;		// Number of samples during attack phase
	DEBUG("Attack sample count: %d", atkCount);
	float step = env.attacklevel / (float) atkCount;	
	DEBUG("Attack step: %f", step);
	for (int i = 0; i < atkCount; i++){			// Attack
		flar.data[i] *= factor;
		factor += step;
		sigIndex++;
		if (sigIndex == BASE_DURATION){
			DEBUG("Left at atk");
			goto finish_release;
		}

	}
	factor = env.attacklevel;
	const int decayCount = env.decay * SAMPLE_SIZE;
	DEBUG("Decay sample count: %d", atkCount);
	step = (env.attacklevel - env.sustainlevel) / (float)decayCount;
	DEBUG("Decay step: %f", step);
	for (int i = 0; i < decayCount; i++){		// Decay
		flar.data[sigIndex] *= factor;
		factor -= step;
		sigIndex++;
		if (sigIndex == BASE_DURATION){
			DEBUG("Left at decay");
			goto finish_release;
		}

	}
	DEBUG("Previous amp level: %f", factor);
	factor = env.sustainlevel;
	while (sigIndex < BASE_DURATION){		// Sustaion
		flar.data[sigIndex] *= factor;
		sigIndex++;
	}

	DEBUG("No interruption");
	finish_release:
	releaseCount = LEFTOVER;			// Release
	step = factor / (float) releaseCount;
	DEBUG("Release step: %f", step);
	DEBUG("Releasecount: %d", releaseCount);
	for (int i = 0; i < releaseCount; i++){
		flar.data[sigIndex] *= factor;
		factor -= step;
		sigIndex++;
	}
	
	DEBUG("Left at release with sigIndex %d and factor %f and TOTAL_DURATION %d", sigIndex, factor+step, TOTAL_DURATION);
	return flar;
}

void initInstruments(void)
{
	INSTRUMENTS[PIANO] = &pianoNote;
}
