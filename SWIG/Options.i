
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

/*
    $Id$
    $Source$
    $Log$
    Revision 1.11  2001/04/09 12:24:58  nando
    updated copyright notice header and improved CVS tags

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
