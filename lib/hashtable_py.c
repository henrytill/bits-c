#include <stdlib.h>

#include <Python.h>

#include "hashtable.h"

static PyObject *py_table_create(PyObject *self, PyObject *args)
{
	size_t columns_len = 0;

	int const rc = PyArg_ParseTuple(args, "n", &columns_len);
	if (rc == 0) {
		return NULL;
	}

	struct table *const t = table_create(columns_len);
	if (t == NULL) {
		PyErr_SetString(PyExc_RuntimeError, "Failed to create table");
		return NULL;
	}

	return PyCapsule_New(t, "hashtable.table", NULL);
}

static void decref(void *value)
{
	Py_XDECREF(value);
}

static PyObject *py_table_destroy(PyObject *self, PyObject *args)
{
	PyObject *py_table = NULL;

	int const rc = PyArg_ParseTuple(args, "O", &py_table);
	if (rc == 0) {
		return NULL;
	}

	struct table *const t = PyCapsule_GetPointer(py_table, "hashtable.table");
	if (t == NULL) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid table capsule");
		return NULL;
	}

	table_destroy(t, decref);
	Py_RETURN_NONE;
}

static PyObject *py_table_put(PyObject *self, PyObject *args)
{
	PyObject   *py_table = NULL;
	char const *key      = NULL;
	PyObject   *value    = NULL;

	int const rc = PyArg_ParseTuple(args, "OsO", &py_table, &key, &value);
	if (rc == 0) {
		return NULL;
	}

	struct table *const t = PyCapsule_GetPointer(py_table, "hashtable.table");
	if (t == NULL) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid table capsule");
		return NULL;
	}

	Py_XINCREF(value); // optimistically increment reference count
	int const result = table_put(t, key, value);
	if (result != 0) {
		Py_XDECREF(value); // decrement reference count if put failed
		PyErr_SetString(PyExc_RuntimeError, "put failed");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *py_table_get(PyObject *self, PyObject *args)
{
	PyObject   *py_table = NULL;
	char const *key      = NULL;

	int const rc = PyArg_ParseTuple(args, "Os", &py_table, &key);
	if (rc == 0) {
		return NULL;
	}

	struct table *const t = PyCapsule_GetPointer(py_table, "hashtable.table");
	if (t == NULL) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid table capsule");
		return NULL;
	}

	PyObject *result = table_get(t, key);
	if (result == NULL) {
		Py_RETURN_NONE;
	}

	Py_INCREF(result);
	return result;
}

static PyMethodDef hashtable_methods[] = {
	{"create", py_table_create, METH_VARARGS, "Create a new table"},
	{"destroy", py_table_destroy, METH_VARARGS, "Destroy an existing table"},
	{"put", py_table_put, METH_VARARGS, "Insert a key-value pair into the table"},
	{"get", py_table_get, METH_VARARGS, "Retrieve a value by key from the table"},
	{NULL, NULL, 0, NULL},
};

static struct PyModuleDef hashtable_module = {
	PyModuleDef_HEAD_INIT,
	"hashtable",
	NULL,
	-1,
	hashtable_methods,
};

PyMODINIT_FUNC PyInit_hashtable(void) // NOLINT(readability-identifier-naming)
{
	return PyModule_Create(&hashtable_module);
}
