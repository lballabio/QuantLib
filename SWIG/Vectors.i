
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

#ifndef quantlib_vectors_i
#define quantlib_vectors_i

%module Vectors

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

%{
// std::vector specializations
#include <vector>
typedef std::vector<int> IntVector;
typedef std::vector<double> DoubleVector;
// array
using QuantLib::Array;
%}

%typemap(python,in) IntVector, IntVector *, const IntVector & {
	if (PyTuple_Check($source)) {
		int size = PyTuple_Size($source);
		$target = new IntVector(size);
		for (int i=0; i<size; i++) {
			PyObject* o = PyTuple_GetItem($source,i);
			if (PyInt_Check(o)) {
				(*$target)[i] = int(PyInt_AsLong(o));
			} else {
				PyErr_SetString(PyExc_TypeError,"tuple must contain integers");
				delete $target;
				return NULL;
			}
		}
	} else if (PyList_Check($source)) {
		int size = PyList_Size($source);
		$target = new IntVector(size);
		for (int i=0; i<size; i++) {
			PyObject* o = PyList_GetItem($source,i);
			if (PyInt_Check(o)) {
				(*$target)[i] = int(PyInt_AsLong(o));
			} else {
				PyErr_SetString(PyExc_TypeError,"list must contain integers");
				delete $target;
				return NULL;
			}
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a sequence");
		return NULL;
	}
};

%typemap(python,freearg) IntVector, IntVector *, const IntVector & {
	delete $source;
};

%typemap(python,out) IntVector, IntVector *, const IntVector & {
	$target = PyTuple_New($source->size());
	for (int i=0; i<$source->size(); i++)
		PyTuple_SetItem($target,i,PyInt_FromLong((*$source)[i]));
};

%typemap(python,ret) IntVector {
	delete $source;
}


%typemap(python,in) DoubleVector, DoubleVector * ,const DoubleVector &, 
                    Array, Array *, const Array & {
	if (PyTuple_Check($source)) {
		int size = PyTuple_Size($source);
		$target = new $basetype(size);
		for (int i=0; i<size; i++) {
			PyObject* o = PyTuple_GetItem($source,i);
			if (PyFloat_Check(o)) {
				(*$target)[i] = PyFloat_AsDouble(o);
			} else if (PyInt_Check(o)) {
				(*$target)[i] = double(PyInt_AsLong(o));
			} else {
				PyErr_SetString(PyExc_TypeError,"tuple must contain doubles");
				delete $target;
				return NULL;
			}
		}
	} else if (PyList_Check($source)) {
		int size = PyList_Size($source);
		$target = new $basetype(size);
		for (int i=0; i<size; i++) {
			PyObject* o = PyList_GetItem($source,i);
			if (PyFloat_Check(o)) {
				(*$target)[i] = PyFloat_AsDouble(o);
			} else if (PyInt_Check(o)) {
				(*$target)[i] = double(PyInt_AsLong(o));
			} else {
				PyErr_SetString(PyExc_TypeError,"list must contain doubles");
				delete $target;
				return NULL;
			}
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a sequence");
		return NULL;
	}
};

%typemap(python,freearg) DoubleVector, DoubleVector *, const DoubleVector &,
                         Array, Array *, const Array & {
	delete $source;
};

%typemap(python,out) DoubleVector, DoubleVector *, const DoubleVector &, 
                     Array, Array *, const Array & {
	$target = PyTuple_New($source->size());
	for (int i=0; i<$source->size(); i++)
		PyTuple_SetItem($target,i,PyFloat_FromDouble((*$source)[i]));
};

%typemap(python,ret) DoubleVector, Array {
	delete $source;
}


#endif
