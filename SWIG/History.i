
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

#ifndef quantlib_history_i
#define quantlib_history_i

%module History

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

%include Date.i
%include Vectors.i
%include String.i

%{
#include "history.h"
%}

// History class

%{
using QuantLib::History;
%}

class History {
  public:
	History(DateVector dates, DoubleVector values);
	~History();
	Date firstDate() const;
	Date lastDate() const;
	int size() const;
};

#if defined(SWIGPYTHON)

%addmethods History {
	String __str__() {
		return "Historical data from " + 
		    DateFormatter::toString(self->firstDate()) +
			" to " + DateFormatter::toString(self->lastDate());
	}
	String __repr__() {
		return "<History: historical data from " + 
		    DateFormatter::toString(self->firstDate()) + 
			" to " + DateFormatter::toString(self->lastDate())+">";
	}
	double __getitem__(Date d) {
		return (*self)[d];
	}
}

#endif


#endif
