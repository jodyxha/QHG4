// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: PyPieAttributes.h
// ****************************************************************************

#ifndef PY_PIEATTRIBUTES_H
#define PY_PIEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <PieAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define PIEATTRIBUTES_NMETH 24
void           PyPieAttributes_StartUp(PieAttributes *subj, void *data);
void           PyPieAttributes_CloseDown();
PyMethodDef *  PyPieAttributes_GetMethodTable(int *nMethods);
bool           PyPieAttributes_Check(PyObject *obj);
PieAttributes *  PyPieAttributes_FromPyObject(PyObject *obj);
PyObject *     PyPieAttributes_New();
PyObject *     PyPieAttributes_Wrap(const PieAttributes *attr);
void           PyPieAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyPieAttributes_SetDefaults(const PieAttributes *atts);
std::string    PyPieAttributes_GetLogString();
std::string    PyPieAttributes_ToString(const PieAttributes *, const char *);
PyObject *     PyPieAttributes_getattr(PyObject *self, char *name);
int            PyPieAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyPieAttributes_methods[PIEATTRIBUTES_NMETH];

#endif

