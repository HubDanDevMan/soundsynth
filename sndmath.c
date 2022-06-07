#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#include <stdio.h>
#include "melodyparser.h"
#include "debug.h"
#include "soundwaves.h"
#include "harmonics.h"
#include "multimel.h"
#include "wav.h"
#include "compressor.h"



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
	floatArray_t result = {NULL, 0};
	unsigned long res;


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
		if (curNote.freq != -1.0f){
			result = joinNotes(result, note);
			DEBUG("freq: %f, duration %f", curNote.freq, curNote.duration);
		}
		noteCounter++;
	}

	DEBUG("Done parsing, cleaning up and returning...");
	res = writeAsWAV(result, output);
	free(result.data);
	fclose(output);
	if (res < 0) {
		PyErr_SetString(PyExc_IOError, "Could not write to file.");
		return NULL;
	}
	fclose(output);
	return PyLong_FromLong(noteCounter);

}

// TODO: Find better Formula for adjusting volume. 
/* create_melodies(melodies, filename, osc) */
static PyObject * create_melodies(PyObject * self, PyObject * args){
	FILE * output;
	Py_ssize_t tuplesize;
	Py_ssize_t currentStringSize;
	PyObject * melodytuple;
	PyObject * currentMelodyStr;
	floatArray_t *melodies = NULL;
	floatArray_t note = {NULL, 0};
	floatArray_t result = {NULL, 0};
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
	// set VOLUME to a reasonable value
	VOLUME = VOLUME / (float)tuplesize / 2.0f;
	DEBUG("TUPLESIZE: %lu", tuplesize);

	// allocate space for all the floatarrays
	melodies = calloc(tuplesize, sizeof(floatArray_t));				// Allocate mem for the melodies, init to 0
	

	for (Py_ssize_t i = 0; i < tuplesize; i++){
		currentMelodyStr = PyTuple_GetItem(melodytuple, i);				// Get item from tuple[i]
		if (!PyUnicode_Check(currentMelodyStr)){					// Error checking
			PyErr_SetString(PyExc_ValueError, "Tuple must contain strings only");
			return NULL;
		}	
		melodystr = PyUnicode_AsUTF8AndSize(currentMelodyStr, &currentStringSize);	// Convert current UTF8 to cstring
		DEBUG("Melodystring: %s", melodystr);
		//Py_XDECREF(currentMelodyStr);
		
		if (melodystr == NULL){								// Check string validity 
			PyErr_SetString(PyExc_ValueError, "Unable to decode String"
					" in tuple, tuple content must be a string");
			fclose(output);								// Clean up and raise error
			//Py_XDECREF(melodytuple);
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
				//Py_XDECREF(melodytuple);
				return NULL;
			}

			else if (curNote.done > PARSING_DONE){
				note = createNote(curNote.duration, curNote.freq, osc);		// Create new Note
				melodies[i] = joinNotes(melodies[i], note);
				DEBUG("Appending note with freq: %f, duration %f", curNote.freq, curNote.duration);
			}

		}
		DEBUG("Synthesized %d samples for current melody", melodies[i].length);

		index = 0;		// reset index
		curNote.done = 0.0f;	// reset status flag

	}


	result = joinwaves(melodies, tuplesize);			// Add all the waves together
	DEBUG("Successfuly joined all samples, freeing melodies");
	
	free(melodies);							// Free melodies
	//Py_XDECREF(melodytuple);
	VOLUME = 0.4f;
	//linearCompressor(result, 0.5f, 0.95f);
	res = writeAsWAV(result, output);
	DEBUG("freeing result sample data...");
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
	FILE * output = NULL;				// Output File
	floatArray_t note = {NULL, 0};		// Output data temporary storage
	floatArray_t result = {NULL, 0};	// Final product	
	unsigned long res;

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
				result = joinNotesInstrument(result, note, instr);
				noteCounter++;
			}
		}
	}
	DEBUG("Location %p, length %d", result.data, result.length);
	//res = fwrite(result.data, sizeof(float), result.length, output);	// Write sample data to disk
	res = writeAsWAV(result, output);
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
