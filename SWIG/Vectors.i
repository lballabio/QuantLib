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
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
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
using QuantLib::Null;
using QuantLib::IntegerFormatter;
using QuantLib::DoubleFormatter;
%}

// typemap Python sequence of ints to std::vector<int>

%{
typedef std::vector<int> IntVector;
%}

class IntVector {
  public:
    ~IntVector();
};

%addmethods IntVector {

    IntVector(const IntVector& v) {
        return new IntVector(v);
    }

    int __len__() {
        return self->size();
    }

    int __getitem__(int i) {
        if (i>=0 && i<self->size()) {
            return (*self)[i];
        } else if (i<0 && -i<=self->size()) {
            return (*self)[self->size()+i];
        } else {
            throw IndexError("IntVector index out of range");
        }
    }

    void __setitem__(int i, int x) {
        if (i>=0 && i<self->size()) {
            (*self)[i] = x;
        } else if (i<0 && -i<=self->size()) {
            (*self)[self->size()+i] = x;
        } else {
            throw IndexError("IntVector index out of range");
        }
    }

    IntVector __getslice__(int i, int j) {
        if (i<0)
            i = self->size()+i;
        if (i<0)
            i = 0;
        if (j<0)
            j = self->size()+j;
        if (j > self->size())
            j = self->size();
        IntVector tmp(j-i);
        std::copy(self->begin()+i,self->begin()+j,tmp.begin());
        return tmp;
    }

    void __setslice__(int i, int j, const IntVector& rhs) {
        if (i<0)
            i = self->size()+i;
        if (i<0)
            i = 0;
        if (j<0)
            j = self->size()+j;
        if (j > self->size())
            j = self->size();
        QL_ENSURE(rhs.size() == j-i, "IntVectors are not resizable");
        std::copy(rhs.begin(),rhs.end(),self->begin()+i);
    }

    String __repr__() {
        String s = "(";
        for (int i=0; i<self->size(); i++) {
            if (i != 0)
                s += ", ";
            s += IntegerFormatter::toString((*self)[i]);
        }
        s += ")";
        return s;
    }

    int __nonzero__() {
        return (self->size() == 0 ? 0 : 1);
    }

}; 

%typemap(python,in) IntVector (IntVector temp), IntVector * (IntVector temp), 
  const IntVector & (IntVector temp), IntVector & (IntVector temp) {
    IntVector* v;
    if (PyTuple_Check($source) || PyList_Check($source)) {
        int size = (PyTuple_Check($source) ? 
            PyTuple_Size($source) :
            PyList_Size($source));
        temp = IntVector(size);
        $target = &temp;
        for (int i=0; i<size; i++) {
            PyObject* o = PySequence_GetItem($source,i);
            if (o == Py_None) {
                (*$target)[i] = Null<int>();
            } else if (PyInt_Check(o)) {
                (*$target)[i] = int(PyInt_AsLong(o));
            } else {
                PyErr_SetString(PyExc_TypeError,"ints expected");
                return NULL;
            }
        }
    } else if ((SWIG_ConvertPtr($source,(void **) &v,
      (swig_type_info *)SWIG_TypeQuery("IntVector *"),0)) != -1) {
        $target = v;
    } else {
        PyErr_SetString(PyExc_TypeError,"IntVector expected");
        return NULL;
    }
};


// typemap Python sequence of doubles to std::vector<double>

%{
typedef std::vector<double> DoubleVector;
%}

class DoubleVector {
  public:
    ~DoubleVector();
};

%addmethods DoubleVector {

    DoubleVector(const DoubleVector& v) {
        return new DoubleVector(v);
    }

    int __len__() {
        return self->size();
    }

    double __getitem__(int i) {
        if (i>=0 && i<self->size()) {
            return (*self)[i];
        } else if (i<0 && -i<=self->size()) {
            return (*self)[self->size()+i];
        } else {
            throw QuantLib::IndexError("DoubleVector index out of range");
        }
    }

    void __setitem__(int i, double x) {
        if (i>=0 && i<self->size()) {
            (*self)[i] = x;
        } else if (i<0 && -i<=self->size()) {
            (*self)[self->size()+i] = x;
        } else {
            throw QuantLib::IndexError("DoubleVector index out of range");
        }
    }

    DoubleVector __getslice__(int i, int j) {
        if (i<0)
            i = self->size()+i;
        if (i<0)
            i = 0;
        if (j<0)
            j = self->size()+j;
        if (j > self->size())
            j = self->size();
        DoubleVector tmp(j-i);
        std::copy(self->begin()+i,self->begin()+j,tmp.begin());
        return tmp;
    }

    void __setslice__(int i, int j, const DoubleVector& rhs) {
        if (i<0)
            i = self->size()+i;
        if (i<0)
            i = 0;
        if (j<0)
            j = self->size()+j;
        if (j > self->size())
            j = self->size();
        QL_ENSURE(rhs.size() == j-i, "DoubleVectors are not resizable");
        std::copy(rhs.begin(),rhs.end(),self->begin()+i);
    }

    String __repr__() {
        String s = "(";
        for (int i=0; i<self->size(); i++) {
            if (i != 0)
                s += ", ";
            s += DoubleFormatter::toString((*self)[i]);
        }
        s += ")";
        return s;
    }

    int __nonzero__() {
        return (self->size() == 0 ? 0 : 1);
    }

}; 

%typemap(python,in) DoubleVector (DoubleVector temp), 
  DoubleVector * (DoubleVector temp), const DoubleVector & (DoubleVector temp), 
  DoubleVector & (DoubleVector temp) {
    DoubleVector* v;
    if (PyTuple_Check($source) || PyList_Check($source)) {
        int size = (PyTuple_Check($source) ? 
            PyTuple_Size($source) :
            PyList_Size($source));
        temp = DoubleVector(size);
        $target = &temp;
        for (int i=0; i<size; i++) {
            PyObject* o = PySequence_GetItem($source,i);
            if (o == Py_None) {
                (*$target)[i] = Null<double>();
            } else if (PyFloat_Check(o)) {
                (*$target)[i] = PyFloat_AsDouble(o);
            } else if (PyInt_Check(o)) {
                (*$target)[i] = double(PyInt_AsLong(o));
            } else {
                PyErr_SetString(PyExc_TypeError,"doubles expected");
                return NULL;
            }
        }
    } else if ((SWIG_ConvertPtr($source,(void **) &v,
      (swig_type_info *)SWIG_TypeQuery("DoubleVector *"),0)) != -1) {
        $target = v;
    } else {
        PyErr_SetString(PyExc_TypeError,"DoubleVector expected");
        return NULL;
    }
};


#endif
