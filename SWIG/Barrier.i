
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
    Revision 1.4  2001/04/09 12:24:58  nando
    updated copyright notice header and improved CVS tags

*/

#ifndef quantlib_barrier_type_i
#define quantlib_barrier_type_i

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
