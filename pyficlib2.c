//-----------------------------------------------------------------------------
// FiC Library 2 Python Wrapper
// nyacom (C) 2018 
//-----------------------------------------------------------------------------
#include "Python.h"
#include "ficlib2.h"

//-----------------------------------------------------------------------------
// Python wrapper
//-----------------------------------------------------------------------------
static PyObject *py_fic_gpio_open(PyObject *self, PyObject *args) {
	if (fic_gpio_open() < 0) {
		return NULL;
	}
	return Py_BuildValue("");
}

static PyObject *py_fic_gpio_close(PyObject *self, PyObject *args) {
	if(fic_gpio_close() < 0) {
		return NULL;
	}
	return Py_BuildValue("");
}

//-----------------------------------------------------------------------------
static PyObject *py_fic_done(PyObject *self, PyObject *args) {
	int ret = fic_done();
	return Py_BuildValue("b", ret);
}

static PyObject *py_fic_power(PyObject *self, PyObject *args) {
	int ret = fic_power();
	return Py_BuildValue("b", ret);
}

//-----------------------------------------------------------------------------
static PyObject *py_fic_prog_sm16(PyObject *self, PyObject *args, PyObject *kwargs) {
	Py_buffer data;
	int pm = 0;

	static char *kwd[] = {"data", "progmode", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*i", kwd, &data, &pm)) {
		return NULL;
	}

	uint32_t ret = fic_prog_sm16(data.buf, data.len, pm, NULL);
	return Py_BuildValue("I", ret);
}

static PyObject *py_fic_prog_sm8(PyObject *self, PyObject *args, PyObject *kwargs) {
	Py_buffer data;
	int pm = 0;

	static char *kwd[] = {"data", "progmode", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*i", kwd, &data, &pm)) {
		return NULL;
	}

	uint32_t ret = fic_prog_sm8(data.buf, data.len, pm, NULL);
	return Py_BuildValue("I", ret);
}

static PyObject *py_fic_prog_init(PyObject *self, PyObject *args) {
	fic_prog_init();
	return Py_BuildValue("");
}

//-----------------------------------------------------------------------------
static PyObject *py_fic_wb8(PyObject *self, PyObject *args, PyObject *kwargs) {
	uint8_t data;
	uint16_t addr;

	static char *kwd[] = {"addr", "data", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Hb", kwd, &addr, &data)) {
		return NULL;
	}

	int ret;
	if ((ret = fic_wb8(addr, data)) < 0) {
		return NULL;
	}

	return Py_BuildValue("");
}

static PyObject *py_fic_rb8(PyObject *self, PyObject *args, PyObject *kwargs) {
	uint16_t addr;

	static char *kwd[] = {"addr", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "H", kwd, &addr)) {
		return NULL;
	}

	int ret;
	if ((ret = fic_rb8(addr)) < 0) {
		return NULL;
	}

	return Py_BuildValue("b", ret & 0xff);
}

static PyObject *py_fic_wb4(PyObject *self, PyObject *args, PyObject *kwargs) {
	uint8_t data;
	uint16_t addr;

	static char *kwd[] = {"addr", "data", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Hb", kwd, &addr, &data)) {
		return NULL;
	}

	int ret;
	if ((ret = fic_wb4(addr, data)) < 0) {
		return NULL;
	}

	return Py_BuildValue("");
}

static PyObject *py_fic_rb4(PyObject *self, PyObject *args, PyObject *kwargs) {
	uint16_t addr;

	static char *kwd[] = {"addr", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "H", kwd, &addr)) {
		return NULL;
	}

	int ret;
	if ((ret = fic_rb4(addr)) < 0) {
		return NULL;
	}

	return Py_BuildValue("b", ret & 0xff);
}

//-----------------------------------------------------------------------------
static PyObject *py_fic_hls_reset8(PyObject *self, PyObject *args) {
	fic_hls_reset8();
	return Py_BuildValue("");
}

static PyObject *py_fic_hls_reset4(PyObject *self, PyObject *args) {
	fic_hls_reset4();
	return Py_BuildValue("");
}

static PyObject *py_fic_hls_start4(PyObject *self, PyObject *args) {
	fic_hls_start4();
	return Py_BuildValue("");
}

static PyObject *py_fic_hls_start8(PyObject *self, PyObject *args) {
	fic_hls_start8();
	return Py_BuildValue("");
}

static PyObject *py_fic_hls_send4(PyObject *self, PyObject *args, PyObject *kwargs) {
	Py_buffer data;

	static char *kwd[] = {"data", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*", kwd, &data)) {
		return NULL;
	}

	int ret;
	if ((ret = fic_hls_send4(data.buf, data.len)) < 0) {
		return NULL;
	}

	return Py_BuildValue("");
}

static PyObject *py_fic_hls_receive4(PyObject *self, PyObject *args, PyObject *kwargs) {
	size_t count;

	static char *kwd[] = {"count", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "H", kwd, &count)) {
		return NULL;
	}

	uint8_t *buf = (uint8_t*) PyObject_Malloc(sizeof(uint8_t)*count);
	if (!buf) {
		return NULL;
	}

	int ret;
	if ((ret = fic_hls_receive4(count, buf)) < 0) {
		return NULL;
	}

	PyObject *array = PyByteArray_FromStringAndSize(buf, count);
	return array;
}
//-----------------------------------------------------------------------------
// Methods
//-----------------------------------------------------------------------------
static PyMethodDef pyficlib2_methods[] = {
	{ "gpio_open",	py_fic_gpio_open, METH_NOARGS, "Open GPIO and create LOCK_FILE"},
	{ "gpio_close",	py_fic_gpio_close, METH_NOARGS, "Close GPIO and delete LOCK_FILE"},
	{ "get_done",	py_fic_done, METH_NOARGS, "Probe DONE singal from FPGA"},
	{ "get_power",	py_fic_power, METH_NOARGS, "Probe PW_OK singal from FiC board"},
	{ "prog_sm16", (PyCFunction) py_fic_prog_sm16, METH_VARARGS|METH_KEYWORDS, "Program FPGA by SMx16 method"},
	{ "prog_sm8", (PyCFunction) py_fic_prog_sm8, METH_VARARGS|METH_KEYWORDS, "Program FPGA by SMx8 method"},
	{ "prog_init", (PyCFunction) py_fic_prog_init, METH_NOARGS, "Reset FPGA"},
	{ "rb8", (PyCFunction) py_fic_rb8, METH_VARARGS|METH_KEYWORDS, "Read byte with 8bit I/F"},
	{ "rb4", (PyCFunction) py_fic_rb4, METH_VARARGS|METH_KEYWORDS, "Read byte with 4bit I/F"},
	{ "wb8", (PyCFunction) py_fic_wb8, METH_VARARGS|METH_KEYWORDS, "Write byte with 8bit I/F"},
	{ "wb4", (PyCFunction) py_fic_wb4, METH_VARARGS|METH_KEYWORDS, "Write byte with 4bit I/F"},
	{ "hls_reset4", (PyCFunction) py_fic_hls_reset4, METH_NOARGS, "Reset HLS module (4bit I/F)"},
	{ "hls_reset8", (PyCFunction) py_fic_hls_reset8, METH_NOARGS, "Reset HLS module (4bit I/F)"},
	{ "hls_start4", (PyCFunction) py_fic_hls_start4, METH_NOARGS, "Start HLS module (4bit I/F)"},
	{ "hls_start8", (PyCFunction) py_fic_hls_start8, METH_NOARGS, "Start HLS module (4bit I/F)"},
	{ "hls_send4", (PyCFunction) py_fic_hls_send4, METH_VARARGS|METH_KEYWORDS, "Send data to HLS with 4bit I/F"},
	{ "hls_receive4", (PyCFunction) py_fic_hls_receive4, METH_VARARGS|METH_KEYWORDS, "Receive data from HLS with 4bit I/F"},
	{ NULL, NULL, 0, NULL },	// Sentinel
};

// Module defs
static struct PyModuleDef pyficlib2 = {
	PyModuleDef_HEAD_INIT,	
	"pyficlib2",// name of module
	"",			// module documents
	-1,
	pyficlib2_methods,
};

PyMODINIT_FUNC PyInit_pyficlib2(void) {
	return PyModule_Create(&pyficlib2);
}
