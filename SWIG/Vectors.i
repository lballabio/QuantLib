
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
%echo "Warning: Vectors is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

%include String.i

%{
#include <vector>
typedef std::vector<int> IntVector;
typedef std::vector<double> DoubleVector;
using QuantLib::Array;
typedef QuantLib::Array PyArray;
typedef QuantLib::Math::LexicographicalView<Array::iterator> ArrayLexicographicalView;
typedef QuantLib::Math::LexicographicalView<Array::iterator>::y_iterator 
	ArrayLexicographicalViewColumn;
using QuantLib::Null;
%}

// array as python shadow class

class Array {
  public:
    Array(int size, double value = 0.0);
    ~Array();
    int size() const;
};

%addmethods Array {
    double __getitem__(int i) {
        return (*self)[i];
    }
    void __setitem__(int i, double x) {
        (*self)[i] = x;
    }
    String __str__() {
        String s = "(";
        for (int i=0; i<self->size(); i++) {
        	if (i != 0)
        		s += ",";
        	s += QuantLib::DoubleFormatter::toString((*self)[i]);
        }
        s += ")";
        return s;
    }
}; 


// 2-D view

class ArrayLexicographicalView {
  public:
    ~ArrayLexicographicalView();
    int xSize() const;
    int ySize() const;
};

%{
    ArrayLexicographicalView CreateLexicographicView(Array& a, int xSize) {
        return ArrayLexicographicalView(a.begin(),a.end(),xSize);
    }
%}

%name(LexicographicalView) ArrayLexicographicalView CreateLexicographicView(Array& a, int xSize);

class ArrayLexicographicalViewColumn {
  public:
    ~ArrayLexicographicalViewColumn();
};

%addmethods ArrayLexicographicalView {
    ArrayLexicographicalViewColumn __getitem__(int i) {
        return (*self)[i];
    }
    String __str__() {
        String s;
        for (int j=0; j<self->ySize(); j++) {
    	    s += "\n";
            for (int i=0; i<self->xSize(); i++) {
                if (i != 0)
                    s += ",";
                s += QuantLib::DoubleFormatter::toString((*self)[i][j]);
            }
        }
        s += "\n";
        return s;
    }
};

%addmethods ArrayLexicographicalViewColumn {
    double __getitem__(int i) {
        return (*self)[i];
    }
    void __setitem__(int i, double x) {
        (*self)[i] = x;
    }
};
    	

// typemaps

%typemap(python,in) IntVector, IntVector *, const IntVector & {
    if (PyTuple_Check($source)) {
        int size = PyTuple_Size($source);
        $target = new IntVector(size);
        for (int i=0; i<size; i++) {
            PyObject* o = PyTuple_GetItem($source,i);
            if (PyInt_Check(o)) {
                (*$target)[i] = int(PyInt_AsLong(o));
            } else if (o == Py_None) {
                (*$target)[i] = Null<int>();
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
            } else if (o == Py_None) {
                (*$target)[i] = Null<int>();
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
                    PyArray, PyArray *, const PyArray & {
    if (PyTuple_Check($source)) {
        int size = PyTuple_Size($source);
        $target = new $basetype(size);
        for (int i=0; i<size; i++) {
            PyObject* o = PyTuple_GetItem($source,i);
            if (PyFloat_Check(o)) {
                (*$target)[i] = PyFloat_AsDouble(o);
            } else if (PyInt_Check(o)) {
                (*$target)[i] = double(PyInt_AsLong(o));
            } else if (o == Py_None) {
                (*$target)[i] = Null<double>();
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
            } else if (o == Py_None) {
                (*$target)[i] = Null<double>();
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
                         PyArray, PyArray *, const PyArray & {
    delete $source;
};

%typemap(python,out) DoubleVector, DoubleVector *, const DoubleVector &, 
                     PyArray, PyArray *, const PyArray & {
    $target = PyTuple_New($source->size());
    for (int i=0; i<$source->size(); i++)
        PyTuple_SetItem($target,i,PyFloat_FromDouble((*$source)[i]));
};

%typemap(python,ret) DoubleVector, PyArray {
    delete $source;
}


#endif
