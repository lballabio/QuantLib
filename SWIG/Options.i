
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
   Revision 1.9  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_options_i
#define quantlib_options_i

%{
using QuantLib::Option;
typedef Option::Type OptionType;
using QuantLib::StringFormatter;
%}

%typemap(python,in) OptionType, const OptionType & {
	if (PyString_Check($source)) {
		$target = new OptionType;
		std::string s(PyString_AsString($source));
		s = StringFormatter::toLowercase(s);
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

%typemap(python,freearg) OptionType, const OptionType & {
	delete $source;
};

%typemap(python,out) OptionType, const OptionType & {
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
