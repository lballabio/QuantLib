
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

/* $Source$
   $Log$
   Revision 1.8  2001/03/21 15:04:06  lballabio
   Started playing with Ruby

   Revision 1.7  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_string_i
#define quantlib_string_i

%{
typedef std::string String;
%}

%typemap(python,in) String, const String & {
    if (PyString_Check($source)) {
        $target = new std::string(PyString_AsString($source));
    } else {
        PyErr_SetString(PyExc_TypeError,"not a string");
        return NULL;
    }
};

%typemap(python,freearg) String, const String & {
    delete $source;
};

%typemap(python,out) String, const String & {
    $target = PyString_FromString($source->c_str());
};

%typemap(python,ret) String, const String & {
    delete $source;
};


%typemap(ruby,in) String, const String & {
    if (TYPE($source) == T_STRING)
        $target = new std::string(STR2CSTR($source));
    else
        rb_raise(rb_eTypeError, "not a string");
};

%typemap(ruby,freearg) String, const String & {
    delete $source;
};

%typemap(ruby,out) String, const String & {
    $target = rb_str_new2($source->c_str());
};

%typemap(ruby,ret) String, const String & {
    delete $source;
};

// typemap Python list of strings to std::vector<std::string>

%{
	typedef std::vector<std::string> StringVector;
%}

%typemap(python,in) StringVector, StringVector *, const StringVector & {
	if (PyTuple_Check($source)) {
		int size = PyTuple_Size($source);
		$target = new std::vector<std::string>(size);
		for (int i=0; i<size; i++) {
			PyObject* o = PyTuple_GetItem($source,i);
			if (PyString_Check(o)) {
				(*$target)[i] = PyString_AsString(o);
			} else {
				PyErr_SetString(PyExc_TypeError,"tuple must contain strings");
				delete $target;
				return NULL;
			}
		}
	} else if (PyList_Check($source)) {
		int size = PyList_Size($source);
		$target = new std::vector<std::string>(size);
		for (int i=0; i<size; i++) {
			PyObject* o = PyList_GetItem($source,i);
			if (PyString_Check(o)) {
				(*$target)[i] = PyString_AsString(o);
			} else {
				PyErr_SetString(PyExc_TypeError,"list must contain strings");
				delete $target;
				return NULL;
			}
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a sequence");
		return NULL;
	}
};

%typemap(python,freearg) StringVector, StringVector *, const StringVector & {
	delete $source;
};


#endif
