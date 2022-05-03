#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#include <stdio.h>
#include "melodyparser.h"
#include "debug.h"
#include "soundwaves.h"
#include "harmonics.h"
//#include "clip.h"
//#include "wav.h"




/* create_melody(melodystr, filename, oscillator) */
static PyObject * create_melody(PyObject * self, PyObject * args)
{
	const char * melody;				// The melody descriptor is a string passed from Python containing notes and their duration
	const char * filename;			// File to be written to
	int osc;
	struct note curNote = {0.0,{0.0}};	// The parsed current note being evaluated
	unsigned long index = 0;		// Current position in the melody string
	unsigned long noteCounter = 0;		// Counts the number of notes parsed
	FILE * output;				// Output File
	floatArray_t note;			// Output data temporary storage


	// Parse the arguments for melody string
	if (!PyArg_ParseTuple(args, "ssi", &melody, &filename, &osc)){
		return NULL;		// Raises Argument error if no arguments supplied
	}
	if (osc >= OSC_COUNT || osc < 0){
		PyErr_Format(PyExc_ValueError, "Invalid oscillator enum: %d",osc);
		return NULL;
	}

	output = fopen(filename, "wb");	

	if (output == NULL){
		PyErr_Format(PyExc_IOError, "Unable to open file %s", filename);
		return NULL;
	}
	while (curNote.done !=  PARSING_DONE){
		curNote = parseNext(&index, melody);			// Parse note

		if (curNote.done < PARSING_DONE){			// Check if error flag (curNote.done) is set
			if (curNote.done == PARSING_ERROR_DURATION){	// Raise an Exception
				PyErr_SetString(PyExc_ValueError, "Incorrect duration format in melody descriptor.");
			} else if (curNote.done == PARSING_ERROR_NOTE){
				PyErr_SetString(PyExc_ValueError, "Incorrect note format in melody descriptor.");
			} else {
				PyErr_SetString(PyExc_ValueError, "Unspecified error occured.");
			}
			return NULL;		// exit C-API and return to Python
		}

		// handle the data and generate soundwaves
		DEBUG("OSC: %d", osc);
		note = createNote(curNote.duration, curNote.freq, osc);	
		if (fwrite(note.data, sizeof(float), note.length, output) < 0){		// append output to file
			PyErr_SetString(PyExc_IOError, "Could not write to file.");
			// clean up
			free(note.data);
			fclose(output);
			return NULL;
		}
		if (curNote.freq != -1.0f){
			DEBUG("freq: %f, duration %f", curNote.freq, curNote.duration);
 			free(note.data);
		}
		noteCounter++;
	}
	DEBUG("Done parsing, cleaning up and returning...");
	fclose(output);
	return PyLong_FromLong(noteCounter);

}


/* create_melodies(melodies, filename, osc) */
static PyObject * create_melodies(PyObject * self, PyObject * args){
	FILE * output;
	Py_ssize_t tuplesize;
	Py_ssize_t currentStringSize;
	PyObject * melodytuple;
	PyObject * currentMelodyStr;
	floatArray_t *melodies;
	floatArray_t result;
	struct note curNote = {0,{0}};
	const char * filename;
	const char * melodystr;
	unsigned long index = 0;
	long res;
	int osc;
	
	if (!PyArg_ParseTuple(args, "Osi", &melodytuple, &filename, &osc)){		// Parse Arguments
		return NULL;
	}

	if (!PyTuple_Check(melodytuple)){						// Check type
		PyErr_SetString(PyExc_ValueError, "Expected tuple as first argument.");
		return NULL;								// Raise error
	}

	output = fopen(filename, "wb");							// Open file
	if (output == NULL){								// Check file
		PyErr_Format(PyExc_IOError, "Unable to open file '%s'", filename);
		return NULL;								// Raise error
	}

	tuplesize = PyTuple_Size(melodytuple);						// Get size of the tuple
	DEBUG("TUPLESIZE: %lu", tuplesize);

	// allocate space for all the floatarrays
	melodies = calloc(tuplesize, sizeof(floatArray_t));				// Allocate mem for the melodies, init to 0
	

	for (Py_ssize_t i = 0; i < tuplesize; i++){
		currentMelodyStr = PyTuple_GetItem(melodytuple, i);				// Get item from tuple[i]
		melodystr = PyUnicode_AsUTF8AndSize(currentMelodyStr, &currentStringSize);	// Convert current UTF8 to cstring
		DEBUG("Melodystring: %s", melodystr);
		Py_DECREF(currentMelodyStr);
		
		if (melodystr == NULL){								// Check string validity 
			PyErr_SetString(PyExc_ValueError, "Unable to decode String"
					" in tuple, tuple content must be a string");
			fclose(output);								// Clean up and raise error
			Py_XDECREF(melodytuple);
			return NULL;
		}

		while (curNote.done !=  PARSING_DONE){
			curNote = parseNext(&index, melodystr);					// Parse note

			if (curNote.done < PARSING_DONE){					// Check if error flag (curNote.done) is set
				if (curNote.done == PARSING_ERROR_DURATION){			// Raise an Exception if thats the case
					PyErr_SetString(PyExc_ValueError,
					"Incorrect duration format in melody descriptor.");
				} else if (curNote.done == PARSING_ERROR_NOTE){
					PyErr_SetString(PyExc_ValueError,
					"Incorrect note format in melody descriptor.");
				} else {
					PyErr_SetString(PyExc_ValueError,
					"Unspecified error occured.");
				}
				fclose(output);							// Clean up and return
				Py_XDECREF(melodytuple);
				return NULL;
			}

			else if (curNote.done > PARSING_DONE){
				result = createNote(curNote.duration, curNote.freq, osc);		// Create new Note
				if (melodies[i].data != NULL){						// Check if this is the first note being synthesized
					melodies[i].data = realloc(melodies[i].data,
						sizeof(float) * (melodies[i].length + result.length));	// Assign more space to melody sample data
					memcpy( &(melodies[i].data[melodies[i].length]),
						result.data, result.length*sizeof(float));		// Append new note samples
					melodies[i].length += result.length;
					DEBUG("Next note, realloced %d floats", result.length);
					free(result.data);						// Free note data as it has been copied to melodies array
				} else {								// For first note there is no need reallocate data
					DEBUG("First note, no reallocation");
					melodies[i].data = result.data;
					melodies[i].length = result.length;
				}
				DEBUG("Appending note with freq: %f, duration %f", curNote.freq, curNote.duration);
			}

		}
		DEBUG("Synthesized %d samples for current melody", melodies[i].length);

		index = 0;		// reset index
		curNote.done = 0.0f;	// reset status flag

	}


	if (DEBUG_STATE){											// Get some debugging information on the melodies
		for (int j = 0; j < tuplesize; j++){
			DEBUG("LENGTH: %d\tLOCATION: %p", melodies[j].length, melodies[j].data);
		}
	}

	result = joinwaves(melodies, tuplesize);			// Add all the waves together
	DEBUG("Successfuly joined all samples");
	
	free(melodies);							// Free melodies
	Py_XDECREF(melodytuple);

	res = fwrite(result.data, sizeof(float), result.length, output);// Write sample data to disk
	free(result.data);
	fclose(output);
	if (res < 0) {
		PyErr_SetString(PyExc_IOError, "Could not write to file.");
		return NULL;
	}
	DEBUG("Successfuly wrote %lu floats to output file, leaving...", res);

	return PyLong_FromLong(tuplesize);	// returns number of melodies added
}

static PyObject * instrument_melody(PyObject * self, PyObject * args)
{
	const char * melodystr;			// The melody descriptor is a string passed from Python containing notes and their duration
	const char * filename;			// File to be written to
	int instr;				// Instrument enum

	struct note curNote = {0.0,{0.0}};	// The parsed current note being evaluated
	unsigned long index = 0;		// Current position in the melody string
	unsigned long noteCounter = 0;		// Counts the number of notes parsed
	FILE * output;				// Output File
	floatArray_t note = {NULL, 0};		// Output data temporary storage
	floatArray_t result = {NULL, 0};	// Final product	
	unsigned long res;
	int reallocSizeBytes;
	int i;

	if (!PyArg_ParseTuple(args, "ssi", &melodystr, &filename, &instr)){
		return NULL;
	}
	if (instr >= INSTRUMENT_END || instr < 0){
		PyErr_Format(PyExc_ValueError, "Invalid instrument enum: %d", instr);
		return NULL;
	}

	output = fopen(filename, "wb");	

	if (output == NULL){
		PyErr_Format(PyExc_IOError, "Unable to open file %s", filename);
		return NULL;
	}

	while (curNote.done != PARSING_DONE){
		curNote = parseNext(&index, melodystr);	
		if (curNote.done < PARSING_DONE){
			if (curNote.done == PARSING_ERROR_DURATION){			// Raise an Exception if thats the case
				PyErr_SetString(PyExc_ValueError,
				"Incorrect duration format in melody descriptor.");
			} else if (curNote.done == PARSING_ERROR_NOTE){
				PyErr_SetString(PyExc_ValueError,
				"Incorrect note format in melody descriptor.");
			} else {
				PyErr_SetString(PyExc_ValueError,
				"Unspecified error occured.");
			}
			fclose(output);							// Clean up and return
			return NULL;
		} else if (curNote.done > PARSING_DONE ){
			note = (*INSTRUMENTS[instr])(curNote.duration, curNote.freq);	
			DEBUG("Note at %p length is %d samples", note.data, note.length);
			if (curNote.freq != -1.0f){
				// check if currentNote is the first one
				if (result.data == NULL){
					result.data = note.data;
					result.length = note.length;
				} else {		// not first note
					int releaseCount = instrdata[instr].env.release * SAMPLE_SIZE;
					// Add to previous release signal
					for (i = 0; i < releaseCount; i++){
						result.data[result.length + i - releaseCount] += note.data[i];
					}

					reallocSizeBytes = (result.length + note.length - releaseCount) * sizeof(float);
					DEBUG("Reallocation size: %d, thats %d new floats", reallocSizeBytes, reallocSizeBytes/4);
					result.data = realloc(result.data, reallocSizeBytes);

					/*	previous note end		current note without prev rel phase	floatcount without previous release phase */
					memcpy( &(result.data[result.length]) , &(note.data[releaseCount]), sizeof(float)*(note.length - releaseCount));
					result.length = result.length + note.length - releaseCount;

					DEBUG("freing note %p with length %d", note.data, note.length);
					free(note.data);
				}
				DEBUG("freq: %f, duration %f", curNote.freq, curNote.duration);
				noteCounter++;
			}
		}
	}
	DEBUG("Location %p, length %d", result.data, result.length);
	res = fwrite(result.data, sizeof(float), result.length, output);	// Write sample data to disk
	free(result.data);
	fclose(output);
	if (res < 0) {
		PyErr_SetString(PyExc_IOError, "Could not write to file.");
		return NULL;
	}
	DEBUG("Successfuly wrote %lu floats to output file, leaving...", res);

	return PyLong_FromLong(noteCounter);
}

static PyMethodDef sndmath_methods[] = {
	/* name, functionptr, type, description */
	{"create_melody", create_melody, METH_VARARGS, "creates a soundfile specidied by music descriptor, writes output to filename as raw file"},
	{"create_melodies", create_melodies, METH_VARARGS, "creates a soundfile specidied by music descriptors, writes output to filename as raw file"},
	{"instrument_melody", instrument_melody, METH_VARARGS, "creates a soundfile specidied by music descriptors, writes output to filename as raw file with an instrument"},
	/* Wrapper functions */
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef sndmathmod = {
	/* MODULE DESCRIPTION, module name, ptr, PyMethodDef */
	PyModuleDef_HEAD_INIT, "sndmath", NULL,-1, sndmath_methods
};

PyMODINIT_FUNC PyInit_sndmath(void){
	// initiate lookup tables used for faster runtime lookup
	initInstruments();			// Initializes the instruments and their soundwave function pointers 
	initNoteValues();
	initOscillators();
	PyObject* module = PyModule_Create(&sndmathmod);
	PyModule_AddIntConstant(module, "SIN", SIN);						// Export enum types for oscillator functions 
	PyModule_AddIntConstant(module, "SAW", SAW);			
	PyModule_AddIntConstant(module, "SQUARE", SQUARE);					
	PyModule_AddIntConstant(module, "SQUAREDSERIES", SQUAREDSERIES);			
	PyModule_AddIntConstant(module, "NOISE", NOISE);					
	PyModule_AddIntConstant(module, "PIANO", PIANO);					
	return module;
}
