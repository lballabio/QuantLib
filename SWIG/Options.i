
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_options_i
#define quantlib_options_i

%module Options

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%{
#include "options.h"
using QuantLib::Option;
typedef Option::Type OptionType;

#include "stringconverters.h"
using QuantLib::ConvertToLowercase;
%}

%typemap(python,in) OptionType, OptionType * {
	if (PyString_Check($source)) {
		$target = new OptionType;
		std::string s(PyString_AsString($source));
		ConvertToLowercase(s);
		if (s == "call")				*($target) = Option::Call;
		else if (s == "put")			*($target) = Option::Put;
		else if (s == "straddle")		*($target) = Option::Straddle;
		else {
			PyErr_SetString(PyExc_TypeError,"unknown option type");
			delete $target;
			return NULL;
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not an option type");
		return NULL;
	}
};

%typemap(python,freearg) OptionType, OptionType * {
	delete $source;
};

%typemap(python,out) OptionType, OptionType * {
	std::string s;
	if (*$source == Option::Call)			s = "call";
	else if (*$source == Option::Put)		s = "put";
	else if (*$source == Option::Straddle)	s = "straddle";
	else s = "unknown option type";
	$target = PyString_FromString(s.c_str());
};

%typemap(python,ret) OptionType {
	delete $source;
}


#endif
