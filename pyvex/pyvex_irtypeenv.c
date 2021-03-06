// This code is GPLed by Yan Shoshitaishvili

#include <Python.h>
#include <structmember.h>
#include <libvex.h>

#include "pyvex_enums.h"
#include "pyvex_types.h"
#include "pyvex_macros.h"

#ifdef PYVEX_STATIC
	#include "pyvex_static.h"
	#include "pyvex_deepcopy.h"
#endif

//////////////////
// Python stuff //
//////////////////

PYVEX_NEW(IRTypeEnv)
PYVEX_DEALLOC(IRTypeEnv)
PYVEX_WRAP(IRTypeEnv)
PYVEX_METH_STANDARD(IRTypeEnv)

static int
pyIRTypeEnv_init(pyIRTypeEnv *self, PyObject *args, PyObject *kwargs)
{
	if (!kwargs) { self->wrapped = PYVEX_COPYOUT(IRTypeEnv, emptyIRTypeEnv()); return 0; }
	PYVEX_WRAP_CONSTRUCTOR(IRTypeEnv);

	PyErr_SetString(VexException, "Unexpected arguments provided.");
	return -1;
}

static PyMemberDef pyIRTypeEnv_members[] =
{
	{NULL}
};

PYVEX_ACCESSOR_WRAPPED(IRTypeEnv, IRTypeEnv, self->wrapped, wrapped, IRTypeEnv)

PyObject *pyIRTypeEnv_types(pyIRTypeEnv *self)
{
	PyObject *result = PyTuple_New(self->wrapped->types_used);
	for (int i = 0; i < self->wrapped->types_used; i++)
	{
		const char *type_str;
		PYVEX_ENUM_TOSTR(IRType, self->wrapped->types[i], type_str, return NULL);

		PyObject *wrapped = PyString_FromString(type_str);
		PyTuple_SetItem(result, i, wrapped);
	}
	return result;
}

PyObject *pyIRTypeEnv_newTemp(pyIRTypeEnv *self, PyObject *type)
{
	IRType t;
	const char *t_str = PyString_AsString(type);
	if (!t_str) { PyErr_SetString(VexException, "Unrecognized type argument to IRTypeEnv.newTemp"); return NULL; }
	PYVEX_ENUM_FROMSTR(IRType, t, t_str, return NULL);

	return PyInt_FromLong(newIRTemp(self->wrapped, t));
}

PyObject *pyIRTypeEnv_typeOf(pyIRTypeEnv *self, PyObject *o)
{
	if (PyInt_Check(o))
	{
		IRTemp t = PyInt_AsLong(o);
		if (t > self->wrapped->types_used || t < 0)
		{
			PyErr_SetString(VexException, "IRTemp out of range.");
			return NULL;
		}

		const char *typestr;
		PYVEX_ENUM_TOSTR(IRType, typeOfIRTemp(self->wrapped, t), typestr, return NULL);
		return PyString_FromString(typestr);
	}
	else if (PyObject_TypeCheck(o, &pyIRExprType))
	{
		pyIRExpr *e = (pyIRExpr *)o;

		const char *typestr;
		PYVEX_ENUM_TOSTR(IRType, typeOfIRExpr(self->wrapped, e->wrapped), typestr, return NULL);
		return PyString_FromString(typestr);
	}

	PyErr_SetString(VexException, "Unrecognized argument to IRTypeEnv.typeOf");
	return NULL;
}

static PyGetSetDef pyIRTypeEnv_getseters[] =
{
	PYVEX_ACCESSOR_DEF(IRTypeEnv, wrapped),
	{NULL}
};

static PyMethodDef pyIRTypeEnv_methods[] =
{
	PYVEX_METHDEF_STANDARD(IRTypeEnv),
	{"types", (PyCFunction)pyIRTypeEnv_types, METH_NOARGS, "Returns a tuple of the IRTypes in the IRTypeEnv"},
	{"newTemp", (PyCFunction)pyIRTypeEnv_newTemp, METH_O, "Creates a new IRTemp in the IRTypeEnv and returns it"},
	{"typeOf", (PyCFunction)pyIRTypeEnv_typeOf, METH_O, "Returns the type of the given IRTemp"},
	{NULL}
};

PYVEX_TYPEOBJECT(IRTypeEnv);
