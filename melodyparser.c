#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "debug.h"
#include "melodyparser.h"

typedef struct {
	char* note;
	float freq;
} noteval_t;

#define MARKER 1.0f
// lookup table for note values
noteval_t NOTE_VALUES[] = {
	{"a2", 0},
	{"a#2", 0},
	{"b2", 0},
	{"c3", 0},
	{"c#3", 0},
	{"d3", 0},
	{"d#3", 0},
	{"e3", 0},
	{"f3", 0},
	{"f#3", 0},
	{"g3", 0} ,
	{"g#3", 0},
	{"a3", 0} ,
	{"a#3", 0},
	{"b3", 0} ,
	{"c4", 0} ,
	{"c#4", 0},
	{"d4", 0} ,
	{"d#4", 0},
	{"e4", 0} ,
	{"f4", 0} ,
	{"f#4", 0},		// 2**(-3/12) * 440
	{"g4", 0} ,		// 2**(-2/12) * 440
	{"g#4", 0},		// 2**(-1/12) * 440
	{"a4", MARKER} , 	// A440, all notes are derived (tuned) from here
	{"a#4", 0},		// 2**(1/12) * 440
	{"b4", 0} ,		// 2**(2/12) * 440
	{"c5", 0} ,
	{"c#5", 0},
	{"d5", 0} ,
	{"d#5", 0},
	{"e5", 0},
	{"f5", 0},
	{"f#5", 0},
	{"g5", 0},
	{"g#5", 0},
	{"a5", 0},
	{"a#5", 0},
	{"b5", 0},
	{"c6", 0},
	{"c#6", 0},
};

#define NOTE_VAL_LEN (sizeof(NOTE_VALUES)/sizeof(NOTE_VALUES[0]))

/* checks if note is valid and determines length of valid note */
static int verifyNote(const char* str){
	DEBUG("Verifying note: %s", str)
	int retval = 0;
	if (*str == 'p'){
		return 1;
	}
	
	switch (tolower(*str)){
		case 'a':
			retval++;
			if (*(str+1) == '#'){
				str++;
				retval++;
			}
			break;
		case 'b': // No b#
			retval++;
			break;
		case 'c':
			retval++;
			if (*(str+1) == '#'){
				str++;
				retval++;
			}
			break;
		case 'd':
			retval++;
			if (*(str+1) == '#'){
				str++;
				retval++;
			}
			break;
		case 'e': // no e#
			retval++;
			break;
		case 'f':
			retval++;
			if (*(str+1) == '#'){
				str++;
				retval++;
			}
			break;
		case 'g':
			retval++;
			if (*(str+1) == '#'){
				str++;
				retval++;
			}
			break;
		default:
			// fprintf(stderr, "Invalid char '%c'\n", tolower(*str));
			// Error display is handled by caller and a Python ValueError Exception is raised
			return -1;

	}
	str++;
	// parse the octave number 
	if (isdigit(*str)){
		return retval+1;
	}
	// If there is no valid octave number, the format is incorrect
	// Error display is handled by caller and a Python ValueError Exception is raised
	return -1;
}

/* determines the frequency of the note via the lookup table */
float lookup(char * notename){
	if (*notename == 'p'){
		return PAUSE_FREQ;
	}
	for (unsigned long i = 0; i < NOTE_VAL_LEN; i++){
		if (strcmp(NOTE_VALUES[i].note, notename) == 0 ){
			return NOTE_VALUES[i].freq; 
		}
	}
	DEBUG("Is %s", NOTE_VALUES[NOTE_VAL_LEN-1].note);
	return PARSING_ERROR_NOTE;
}


static float freqAt(float offset){
	return pow(2.0f, offset/12.0f)*440.0f;
}

/* initiates the note-frequency table */
void initNoteValues(void){
	int start = 0;
	// find index of A440, which has the frequency set as MARKER 
	while (NOTE_VALUES[start].freq != MARKER){
		start++;
	}

	start = 0-start; // distance from start to A440
	for (unsigned long i = 0; i < NOTE_VAL_LEN; i++){
		NOTE_VALUES[i].freq = freqAt((float) start);
		start++;
	}
}

/* Parse the next note frequency and duration from melody string */
struct note parseNext(unsigned long *index, const char* melodystr){
	struct note n;			// note 
	char notebuf[4] = {0,0,0,0}; 	// buffer holding current note name
	int val;			// lenght of note

	// skip whitespace if present
	while (isspace(melodystr[*index])){
		(*index)++;
	}

	// Check if all data has been parsed
	if (strlen(&melodystr[*index]) == 0){
		n.done = PARSING_DONE;
		return n; // No more data to be parsed
	}
	


	// Parse the duration
	n.duration = atof(&melodystr[*index]);
	DEBUG("duration: %f",n.duration);
	if (n.duration <= 0.0f){
		// INVALID DURATION SPECIFIER
		fprintf(stderr, "Invalid duration specifier in:\n%s\n^", &melodystr[*index]);
		n.done = PARSING_ERROR_DURATION;
		return n;
	}
	
	// increment the index until the next character is not in "0123456789."
	while (isdigit(melodystr[*index]) || melodystr[*index] == '.' || isspace(melodystr[*index])){
		DEBUG("skipped '%c'", melodystr[*index]);
		(*index)++;
	}
	
	val = verifyNote(&melodystr[*index]);
	// check if note is invalid
	if (val == -1){
		fprintf(stderr, "Invalid note specifier in:\n%s\n^", &melodystr[*index]);
		n.done = PARSING_ERROR_NOTE;
		return n;
	}


	// else parse the note and look up the frequency
	for (int i = 0; i < val; i++){
		notebuf[i] = tolower(melodystr[*index]);
		DEBUG("NOTEBUF: %c", notebuf[i]);
		(*index)++;
	}


	n.freq = lookup(notebuf);
	return n;
}
