
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/* $Source$
   $Log$
   Revision 1.23  2001/04/06 18:46:21  nando
   changed Authors, Contributors, Licence and copyright header

   Revision 1.22  2001/03/30 15:45:42  lballabio
   Still working on make dist (and added IntVector and DoubleVector to Ruby module)

   Revision 1.21  2001/03/15 10:30:48  lballabio
   Added dummy returns to avoid warnings

   Revision 1.20  2001/03/12 12:59:01  marmar
   __str__ now represents the object while __repr__ is unchanged

   Revision 1.19  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_vectors_i
#define quantlib_vectors_i

%include String.i

%{
#include <vector>
typedef std::vector<int> IntVector;
typedef std::vector<double> DoubleVector;
using QuantLib::Null;
using QuantLib::IntegerFormatter;
using QuantLib::DoubleFormatter;
%}

%typemap(ruby,in) VALUE {
    $target = $source;
};

class IntVector {
  public:
    ~IntVector();
};

%addmethods IntVector {
    #if defined(SWIGRUBY)
    void crash() {}
    IntVector(VALUE v) {
    	if (rb_obj_is_kind_of(v,rb_cArray)) {
            int size = RARRAY(v)->len;
            IntVector* temp = new IntVector(size);
            for (int i=0; i<size; i++) {
                VALUE o = RARRAY(v)->ptr[i];
                if (o == Qnil)
                    (*temp)[i] = Null<int>();
                else if (FIXNUM_P(o))
                    (*temp)[i] = FIX2INT(o);
                else
                    rb_raise(rb_eTypeError,
                        "wrong argument type (expected integers)");
            }
            return temp;
        } else {
            rb_raise(rb_eTypeError,
                "wrong argument type (expected array)");
        }
    }
    #endif
    #if defined(SWIGPYTHON)
    IntVector(const IntVector& v) {
        return new IntVector(v);
    }
    #endif
    #if defined(SWIGPYTHON) || defined(SWIGRUBY)
    String __str__() {
        String s = "(";
        for (int i=0; i<self->size(); i++) {
            if (i != 0)
                s += ", ";
            s += IntegerFormatter::toString((*self)[i]);
        }
        s += ")";
        return s;
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
        QL_DUMMY_RETURN(0)
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
    #endif
    #if defined(SWIGPYTHON)
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
    int __nonzero__() {
        return (self->size() == 0 ? 0 : 1);
    }
    #endif
    #if defined(SWIGRUBY)
    void each() {
        for (int i=0; i<self->size(); i++)
            rb_yield(INT2NUM((*self)[i]));
    }
    #endif
};

// typemap Python sequence of ints to std::vector<int>

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


class DoubleVector {
  public:
    ~DoubleVector();
};

%addmethods DoubleVector {
    #if defined(SWIGRUBY)
    void crash() {}
    DoubleVector(VALUE v) {
    	if (rb_obj_is_kind_of(v,rb_cArray)) {
            int size = RARRAY(v)->len;
            DoubleVector* temp = new DoubleVector(size);
            for (int i=0; i<size; i++) {
                VALUE o = RARRAY(v)->ptr[i];
                if (o == Qnil)
                    (*temp)[i] = Null<int>();
                else if (FIXNUM_P(o))
                    (*temp)[i] = double(FIX2INT(o));
                else if (TYPE(o) == T_FLOAT)
                    (*temp)[i] = NUM2DBL(o);
                else
                    rb_raise(rb_eTypeError,
                        "wrong argument type (expected integers)");
            }
            return temp;
        } else {
            rb_raise(rb_eTypeError,
                "wrong argument type (expected array)");
        }
    }
    #endif
    #if defined(SWIGPYTHON)
    DoubleVector(const DoubleVector& v) {
        return new DoubleVector(v);
    }
    #endif
    #if defined(SWIGPYTHON) || defined(SWIGRUBY)
    String __str__() {
        String s = "(";
        for (int i=0; i<self->size(); i++) {
            if (i != 0)
                s += ", ";
            s += DoubleFormatter::toString((*self)[i]);
        }
        s += ")";
        return s;
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
            throw IndexError("DoubleVector index out of range");
        }
        QL_DUMMY_RETURN(0.0)
    }
    void __setitem__(int i, double x) {
        if (i>=0 && i<self->size()) {
            (*self)[i] = x;
        } else if (i<0 && -i<=self->size()) {
            (*self)[self->size()+i] = x;
        } else {
            throw IndexError("DoubleVector index out of range");
        }
    }
    #endif
    #if defined(SWIGPYTHON)
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
    int __nonzero__() {
        return (self->size() == 0 ? 0 : 1);
    }
    #endif
    #if defined(SWIGRUBY)
    void each() {
        for (int i=0; i<self->size(); i++)
            rb_yield(rb_float_new((*self)[i]));
    }
    #endif
};

// typemap Python sequence of doubles to std::vector<double>

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














