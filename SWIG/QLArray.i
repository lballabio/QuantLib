/*
 * Copyright (C) 2000, 2001
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

/* $Source$
   $Log$
   Revision 1.8  2001/03/15 10:30:48  lballabio
   Added dummy returns to avoid warnings

   Revision 1.7  2001/03/12 12:59:01  marmar
   __str__ now represents the object while __repr__ is unchanged

   Revision 1.6  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_array_i
#define quantlib_array_i

%include String.i

%{
using QuantLib::Array;
typedef QuantLib::Math::LexicographicalView<Array::iterator> 
    ArrayLexicographicalView;
typedef QuantLib::Math::LexicographicalView<Array::iterator>::y_iterator 
	ArrayLexicographicalViewColumn;
%}

// array as python shadow class

class Array {
  public:
    ~Array();
};

%addmethods Array {
    // constructor
    Array(const Array& a) {
        return new Array(a);
    }
    #if defined(SWIGPYTHON)
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
            throw IndexError("Array index out of range");
        }
        QL_DUMMY_RETURN(0.0)
    }
    void __setitem__(int i, double x) {
        if (i>=0 && i<self->size()) {
            (*self)[i] = x;
        } else if (i<0 && -i<=self->size()) {
            (*self)[self->size()+i] = x;
        } else {
            throw IndexError("Array index out of range");
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
    int __nonzero__() {
        return (self->size() == 0 ? 0 : 1);
    }
    #endif
}; 


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
                    "doubles expected");
                return NULL;
            }
        }
    } else if ((SWIG_ConvertPtr($source,(void **) &v,
      (swig_type_info *)SWIG_TypeQuery("Array *"),0)) != -1) {
        $target = v;
    } else {
        PyErr_SetString(PyExc_TypeError,"Array expected");
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
    #if defined(SWIGPYTHON)
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
    #endif
};

%addmethods ArrayLexicographicalViewColumn {
    #if defined(SWIGPYTHON)
    double __getitem__(int i) {
        return (*self)[i];
    }
    void __setitem__(int i, double x) {
        (*self)[i] = x;
    }
    #endif
};



#endif
