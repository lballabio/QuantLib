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

#ifndef quantlib_array_i
#define quantlib_array_i

%module Array

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: QLArray is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

%include String.i

%{
using QuantLib::Array;
typedef QuantLib::Math::LexicographicalView<Array::iterator> 
    ArrayLexicographicalView;
typedef QuantLib::Math::LexicographicalView<Array::iterator>::y_iterator 
	ArrayLexicographicalViewColumn;
using QuantLib::Null;
using QuantLib::IsNull;
%}

// array as python shadow class

class Array {
  public:
    // hide constructor - Arrays can be initially built as 
    // Python sequences
    ~Array();
};

%addmethods Array {
    // sequence methods
    int __len__() {
        return self->size();
    }
    double __getitem__(int i) {
        if (i>=0 && i<self->size()) {
            return (*self)[i];
        } else if (i<0 && -i<=self->size()) {
            return (*self)[self->size()+i];
        } else {
            throw QuantLib::IndexError("Array index out of range");
        }
    }
    void __setitem__(int i, double x) {
        if (i>=0 && i<self->size()) {
            (*self)[i] = x;
        } else if (i<0 && -i<=self->size()) {
            (*self)[self->size()+i] = x;
        } else {
            throw QuantLib::IndexError("Array index out of range");
        }
    }
    Array __getslice__(int i, int j) {
        if (i<0)
            i = self->size()+i;
        if (j<0)
            j = self->size()+j;
        i = QL_MAX(0,i);
        j = QL_MIN(self->size(),j);
        Array tmp(j-i);
        std::copy(self->begin()+i,self->begin()+j,tmp.begin());
        return tmp;
    }
    void __setslice__(int i, int j, const Array& rhs) {
        if (i<0)
            i = self->size()+i;
        if (j<0)
            j = self->size()+j;
        i = QL_MAX(0,i);
        j = QL_MIN(self->size(),j);
        QL_ENSURE(rhs.size() == j-i, "Arrays are not resizable");
        std::copy(rhs.begin(),rhs.end(),self->begin()+i);
    }
    String __str__() {
        String s = "(";
        for (int i=0; i<self->size(); i++) {
        	if (i != 0)
        		s += ", ";
        	s += QuantLib::DoubleFormatter::toString((*self)[i]);
        }
        s += ")";
        return s;
    }
    String __repr__() {
        String s = "(";
        for (int i=0; i<self->size(); i++) {
        	if (i != 0)
        		s += ", ";
        	s += QuantLib::DoubleFormatter::toString((*self)[i]);
        }
        s += ")";
        return s;
    }
    int __nonzero__() {
        return (self->size() == 0 ? 0 : 1);
    }
}; 


// typemaps

%typemap(python,in) Array (Array temp), Array * (Array temp), 
  const Array & (Array temp), Array & (Array temp) {
    Array* v;
    if (PyTuple_Check($source) || PyList_Check($source)) {
        int size = (PyTuple_Check($source) ? 
            PyTuple_Size($source) :
            PyList_Size($source));
        temp = Array(size);
        $target = &temp;
        for (int i=0; i<size; i++) {
            PyObject* o = PySequence_GetItem($source,i);
            if (PyFloat_Check(o)) {
                (*$target)[i] = PyFloat_AsDouble(o);
            } else if (PyInt_Check(o)) {
                (*$target)[i] = double(PyInt_AsLong(o));
            } else {
                PyErr_SetString(PyExc_TypeError,
                    "sequence must contain doubles");
                return NULL;
            }
        }
    } else if ((SWIG_ConvertPtr($source,(void **) &v,
      (swig_type_info *)SWIG_TypeQuery("Array *"),0)) != -1) {
        $target = v;
    } else {
        PyErr_SetString(PyExc_TypeError,"not a sequence");
        return NULL;
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



#endif
