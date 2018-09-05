//-----------------------------------------------------------------------------
// FiC Library 2 Python Wrapper
// nyacom (C) 2018 
//-----------------------------------------------------------------------------
#include "Python.h"
#include "ficlib2.h"

//-----------------------------------------------------------------------------
// Python wrapper
//-----------------------------------------------------------------------------
PyObject *py_fic_gpio_open(PyObject *self, PyObject *args) {
    fic_gpio_open();
	return Py_BuildValue("");
}

PyObject *py_fic_gpio_open(PyObject *self, PyObject *args) {
}

PyObject *py_set_input(PyObject *self, PyObject *args) {
	int g;

	if (!PyArg_ParseTuple(args, "i", &g))
		return NULL;

	DEBUGOUT("SET INPUT %d\n", g);
	INP_GPIO(g);

	return Py_BuildValue("");
}

PyObject *py_set_output(PyObject *self, PyObject *args) {
	int g;

	if (!PyArg_ParseTuple(args, "i", &g))
		return NULL;

	DEBUGOUT("SET OUTPUT %d\n", g);
	INP_GPIO(g);
	OUT_GPIO(g);

	return Py_BuildValue("");
}

PyObject *py_bus_set(PyObject *self, PyObject *args) {
	int v;

	if (!PyArg_ParseTuple(args, "i", &v))
		return NULL;

	DEBUGOUT("SET BUS OUT 0x%x\n", v);
	GPIO_SET = v & 0x0fffffff;

	return Py_BuildValue("");
}

PyObject *py_bus_clear(PyObject *self, PyObject *args) {
	int v;

	if (!PyArg_ParseTuple(args, "i", &v))
		return NULL;

	DEBUGOUT("SET BUS CLEAR 0x%x\n", v);
	GPIO_CLR = v & 0x0fffffff;

	return Py_BuildValue("");
}

PyObject *py_get(PyObject *self, PyObject *args) {
	int g, v;

	if (!PyArg_ParseTuple(args, "i", &g))
		return NULL;

	v = GET_GPIO(g);

	return Py_BuildValue("i", (v ? 1 : 0));
}

PyObject *py_get_bus(PyObject *self, PyObject *args) {
	return Py_BuildValue("i", GET_GPIO_BUS & 0x0fffffff);
}

// Methods
static PyMethodDef rawgpiomodule_methods[] = {
	{ "setup",	py_setup_io,	METH_NOARGS },
	{ "set_input",	py_set_input,	METH_VARARGS },
	{ "set_output",	py_set_output,	METH_VARARGS },
	{ "bus_set",	py_bus_set,	METH_VARARGS },
	{ "bus_clear",	py_bus_clear,	METH_VARARGS },
	{ "get",	py_get,		METH_VARARGS },
	{ "get_bus",	py_get_bus,	METH_NOARGS },
	{ NULL },
};

// Module defs
static struct PyModuleDef rawrpigpio = {
	PyModuleDef_HEAD_INIT,	
	"rawgpiomodule",	// name of module
	"",			// module documents
	-1,
	rawgpiomodule_methods,
};

PyMODINIT_FUNC PyInit_rawrpigpio(void) {
	return PyModule_Create(&rawrpigpio);
}
*/

