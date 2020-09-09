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
	int fd_lock = 0;
	fd_lock = fic_gpio_open();
	if (fd_lock < 0) {
		return NULL;
	}
	return Py_BuildValue("i", fd_lock);
}

static PyObject *py_fic_gpio_close(PyObject *self, PyObject *args, PyObject *kwargs) {
	int fd_lock;
	static char *kwd[] = {"fd_lock", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwd, &fd_lock)) {
		return NULL;
	}

	if(fic_gpio_close(fd_lock) < 0) {
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
static PyObject *py_fic_prog_status(PyObject *self) {
	return Py_BuildValue("iiillll",
		PROG_ASYNC_STATUS.stat,
		PROG_ASYNC_STATUS.smap_mode,
		PROG_ASYNC_STATUS.prog_mode,
		PROG_ASYNC_STATUS.prog_st_time,
		PROG_ASYNC_STATUS.prog_ed_time,
		PROG_ASYNC_STATUS.prog_size,
		PROG_ASYNC_STATUS.tx_size
	);
}

//-----------------------------------------------------------------------------
static PyObject *py_fic_prog_sm16_async(PyObject *self, PyObject *args, PyObject *kwargs) {
	Py_buffer data;
	int pm = 0;

	static char *kwd[] = {"data", "progmode", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*i", kwd, &data, &pm)) {
		PyBuffer_Release(&data);
		return NULL;
	}

	int32_t ret = fic_prog_sm16_async(data.buf, data.len, pm);
	if (ret < 0) {
		return NULL;
	}

	PyBuffer_Release(&data);

	return Py_BuildValue("");
}

static PyObject *py_fic_prog_sm16_fast(PyObject *self, PyObject *args, PyObject *kwargs) {
	Py_buffer data;
	int pm = 0;

	static char *kwd[] = {"data", "progmode", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*i", kwd, &data, &pm)) {
		PyBuffer_Release(&data);
		return NULL;
	}

	uint32_t ret = fic_prog_sm16_fast(data.buf, data.len, pm);
	if (ret < (uint32_t)data.len) {
		PyBuffer_Release(&data);
		return NULL;
	}

	PyBuffer_Release(&data);

	return Py_BuildValue("I", ret);
}

static PyObject *py_fic_prog_sm16(PyObject *self, PyObject *args, PyObject *kwargs) {
	Py_buffer data;
	int pm = 0;

	static char *kwd[] = {"data", "progmode", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*i", kwd, &data, &pm)) {
		PyBuffer_Release(&data);
		return NULL;
	}

		uint32_t ret = fic_prog_sm16(data.buf, data.len, pm);
	if (ret < (uint32_t)data.len) {
		PyBuffer_Release(&data);
		return NULL;
	}

	PyBuffer_Release(&data);

	return Py_BuildValue("I", ret);
}

static PyObject *py_fic_prog_sm8_async(PyObject *self, PyObject *args, PyObject *kwargs) {
	Py_buffer data;
	int pm = 0;

	static char *kwd[] = {"data", "progmode", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*i", kwd, &data, &pm)) {
		PyBuffer_Release(&data);
		return NULL;
	}

	int32_t ret = fic_prog_sm8_async(data.buf, data.len, pm);
	if (ret < 0) {
		return NULL;
	}

	PyBuffer_Release(&data);

	return Py_BuildValue("");
}

static PyObject *py_fic_prog_sm8_fast(PyObject *self, PyObject *args, PyObject *kwargs) {
	Py_buffer data;
	int pm = 0;

	static char *kwd[] = {"data", "progmode", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*i", kwd, &data, &pm)) {
		PyBuffer_Release(&data);
		return NULL;
	}

	uint32_t ret = fic_prog_sm8_fast(data.buf, data.len, pm);
	if (ret < data.len) {
		PyBuffer_Release(&data);
		return NULL;
	}

	PyBuffer_Release(&data);

	return Py_BuildValue("I", ret);
}

static PyObject *py_fic_prog_sm8(PyObject *self, PyObject *args, PyObject *kwargs) {
	Py_buffer data;
	int pm = 0;

	static char *kwd[] = {"data", "progmode", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*i", kwd, &data, &pm)) {
		PyBuffer_Release(&data);
		return NULL;
	}

	uint32_t ret = fic_prog_sm8(data.buf, data.len, pm);
	if (ret < data.len) {
		PyBuffer_Release(&data);
		return NULL;
	}

	PyBuffer_Release(&data);

	return Py_BuildValue("I", ret);
}

static PyObject *py_fic_prog_init(PyObject *self, PyObject *args) {
	fic_prog_init(PM_NORMAL);
	return Py_BuildValue("");
}

//-----------------------------------------------------------------------------
static PyObject *py_fic_write(PyObject *self, PyObject *args, PyObject *kwargs) {
	uint16_t addr;
	uint16_t data;

	static char *kwd[] = {"addr", "data", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Hb", kwd, &addr, &data)) {
		return NULL;
	}

	int ret;
	if ((ret = fic_write(addr, data)) < 0) {
		return NULL;
	}

	return Py_BuildValue("");
}

static PyObject *py_fic_read(PyObject *self, PyObject *args, PyObject *kwargs) {
	uint16_t addr;

	static char *kwd[] = {"addr", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "H", kwd, &addr)) {
		return NULL;
	}

	int ret;
	if ((ret = fic_read(addr)) < 0) {
		return NULL;
	}

	return Py_BuildValue("b", ret);
}

//-----------------------------------------------------------------------------
static PyObject *py_fic_hls_reset(PyObject *self, PyObject *args) {
	fic_hls_reset();
	return Py_BuildValue("");
}

static PyObject *py_fic_hls_start(PyObject *self, PyObject *args) {
	fic_hls_start();
	return Py_BuildValue("");
}

static PyObject *py_fic_hls_send(PyObject *self, PyObject *args, PyObject *kwargs) {
	Py_buffer data;

	static char *kwd[] = {"data", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*", kwd, &data)) {
		PyBuffer_Release(&data);
		return NULL;
	}

	int ret;
	if ((ret = fic_hls_send(data.buf, data.len)) < 0) {
		PyBuffer_Release(&data);
		return NULL;
	}

	PyBuffer_Release(&data);

	return Py_BuildValue("");
}

static PyObject *py_fic_hls_receive(PyObject *self, PyObject *args, PyObject *kwargs) {
	size_t size;

	static char *kwd[] = {"size", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "I", kwd, &size)) {
		return NULL;
	}

	uint8_t *buf = (uint8_t*) PyObject_Malloc(sizeof(uint8_t)*size);
	if (!buf) {
		return NULL;
	}

	int ret;
	if ((ret = fic_hls_receive(buf, size)) < 0) {
		return NULL;
	}

	PyObject *array = PyByteArray_FromStringAndSize(buf, size);
	return array;
}
//-----------------------------------------------------------------------------
// Methods
//-----------------------------------------------------------------------------
static PyMethodDef pyficlib2_methods[] = {
	{ "gpio_open",	 py_fic_gpio_open, METH_NOARGS, "Open GPIO and create LOCK_FILE"},
	{ "gpio_close",	 (PyCFunction) py_fic_gpio_close, METH_VARARGS|METH_KEYWORDS, "Close GPIO and delete LOCK_FILE"},
	{ "get_done",	 py_fic_done, METH_NOARGS, "Probe DONE singal from FPGA"},
	{ "get_power",   py_fic_power, METH_NOARGS, "Probe PW_OK singal from FiC board"},
	{ "prog_sm16",   (PyCFunction) py_fic_prog_sm16, METH_VARARGS|METH_KEYWORDS, "Program FPGA by SMx16 method"},
	{ "prog_sm8",    (PyCFunction) py_fic_prog_sm8, METH_VARARGS|METH_KEYWORDS, "Program FPGA by SMx8 method"},
	{ "prog_sm16_fast",   (PyCFunction) py_fic_prog_sm16_fast, METH_VARARGS|METH_KEYWORDS, "Program FPGA by SMx16 method (Fast mode)"},
	{ "prog_sm8_fast",    (PyCFunction) py_fic_prog_sm8_fast, METH_VARARGS|METH_KEYWORDS, "Program FPGA by SMx8 method (Fast mode)"},
	{ "prog_sm16_async",(PyCFunction) py_fic_prog_sm16_async, METH_VARARGS|METH_KEYWORDS, "Program FPGA by SMx16 method (async)"},
	{ "prog_sm8_async", (PyCFunction) py_fic_prog_sm8_async, METH_VARARGS|METH_KEYWORDS, "Program FPGA by SMx8 method (async)"},
    { "prog_status", (PyCFunction) py_fic_prog_status, METH_NOARGS, "Get Program status"},
	{ "prog_init",   (PyCFunction) py_fic_prog_init, METH_NOARGS, "Reset FPGA"},
	{ "read",        (PyCFunction) py_fic_read, METH_VARARGS|METH_KEYWORDS, "Read byte at address"},
	{ "write",       (PyCFunction) py_fic_write, METH_VARARGS|METH_KEYWORDS, "Write byte at address"},
	{ "hls_reset",   (PyCFunction) py_fic_hls_reset, METH_NOARGS, "Send Reset CMD to HLS module"},
	{ "hls_start",   (PyCFunction) py_fic_hls_start, METH_NOARGS, "Send Start CMD to HLS module"},
	{ "hls_send",    (PyCFunction) py_fic_hls_send, METH_VARARGS|METH_KEYWORDS, "Send data to HLS module"},
	{ "hls_receive", (PyCFunction) py_fic_hls_receive, METH_VARARGS|METH_KEYWORDS, "Receive data from HLS module"},
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
