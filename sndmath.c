#include <math.h>
#include <Python.h>
#include <stdio.h>
#include "soundwaves.h"

static PyObject * helloFromC(PyObject * self, PyObject * args){
	
	float x = sin(3.0);
	printf("Hello from C: %f!\n", x);
	Py_RETURN_NONE;
}

static PyObject * mySin(PyObject * self, PyObject * args){
	// parsing arguments
	float myf;
	if (!PyArg_ParseTuple(args, "f", &myf)){
		return NULL;
	}
	
	return PyFloat_FromDouble(sin(myf));
}
static PyMethodDef sndmath_methods[] = {
	/* name, functionptr, type, description */
	{"helloFromC", helloFromC, METH_VARARGS, "prints hello"},
	{"mySin", mySin, METH_VARARGS, "calculate sine"},
	/* Wrapper functions */
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef helloC = {
	/* MODULE DESCRIPTION, module name, ptr, PyMethodDef */
	PyModuleDef_HEAD_INIT, "sndmath", NULL,-1, sndmath_methods
};

PyMODINIT_FUNC PyInit_sndmath(void){
	return PyModule_Create(&helloC);
}
