/*
 * Copyright (C) 2001
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
 *   http://quantlib.sourceforge.net/LICENSE.TXT
*/

#ifndef quantlib_barrier_type_i
#define quantlib_barrier_type_i

%module Barrier

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: Options is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

%{
using QuantLib::Pricers::BarrierOption;
typedef QuantLib::Pricers::BarrierOption::BarrierType BarrierType;
using QuantLib::StringFormatter;
%}

%typemap(python,in) BarrierType, const BarrierType & {
	if (PyString_Check($source)) {
		$target = new BarrierType;
		std::string s(PyString_AsString($source));
		s = StringFormatter::toLowercase(s);
		if (s == "downin")			*($target) = BarrierOption::DownIn;
		else if (s == "downout")	*($target) = BarrierOption::DownOut;
		else if (s == "upin")		*($target) = BarrierOption::UpIn;
		else if (s == "upout")		*($target) = BarrierOption::UpOut;
		else {
			PyErr_SetString(PyExc_TypeError,"unknown barrier type");
			delete $target;
			return NULL;
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a barrier type");
		return NULL;
	}
};

%typemap(python,freearg) BarrierType, const BarrierType & {
	delete $source;
};

%typemap(python,out) BarrierType, const BarrierType & {
	std::string s;
	if (*$source == BarrierOption::DownIn)			s = "DownIn";
	else if (*$source == BarrierOption::DownOut)	s = "DownOut";
	else if (*$source == BarrierOption::UpIn)	    s = "UpIn";
	else if (*$source == BarrierOption::UpOut)	    s = "UpOut";
	else s = "unknown barrier type";
	$target = PyString_FromString(s.c_str());
};

%typemap(python,ret) BarrierType {
	delete $source;
}

#endif
