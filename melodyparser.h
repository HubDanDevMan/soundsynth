#ifndef _MELODYPARSER_H
#define _MELODYPARSER_H

#include <stdlib.h>
#include <stdio.h>
//#include "harmonics.h"
#define PARSING_DONE 		(-1.0f)
#define PARSING_ERROR_NOTE 	(-2.0f)
#define PARSING_ERROR_DURATION 	(-3.0f)
#define PAUSE_FREQ 0.00001f
struct note {
	float duration;
	union {
		float freq;			// POSITIVE, >0
		float done;			// NEGATIVE  <0
	};
};

void initNoteValues(void);
struct note parseNext(unsigned long *index, const char* melodystr);


/* _MELODYPARSER_H */
#endif
