
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

#ifndef quantlib_instruments_i
#define quantlib_instruments_i

%module Instruments

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

%include Financial.i
%include TermStructures.i
%include String.i

%{
#include <cstdlib>
using QuantLib::Instrument;
using QuantLib::Handle;
typedef Handle<Instrument> InstrumentHandle;

std::string Representation(const Handle<Instrument>& i) {
	std::string isin = i->isinCode();
	if (isin == "")
		isin = "unknown";
	std::string desc = i->description();
	if (desc == "")
		desc = "no description available";
	return (isin+" ("+desc+")");
}
%}

// Export Handle<Instrument>
%name(Instrument) class InstrumentHandle {
  public:
	// no constructor - forbid explicit construction
	~InstrumentHandle();
};

// replicate the Instrument interface
%addmethods InstrumentHandle {
	void setPrice(double price) {
		(*self)->setPrice(price);
	}
	void setTermStructure(TermStructureHandle h) {
		(*self)->setTermStructure(h);
	}
	String isinCode() {
		return (*self)->isinCode();
	}
	String description() {
		return (*self)->description();
	}
	TermStructureHandle termStructure() {
		return (*self)->termStructure();
	}
	double NPV() {
		return (*self)->NPV();
	}
	double price() {
		return (*self)->price();
	}
	#if defined(SWIGPYTHON)
	String __str__() {
		return Representation(*self);
	}
	String __repr__() {
		return "<Handle<Instrument>: "+Representation(*self)+">";
	}
	int __cmp__(const InstrumentHandle& other) {
		return ((*self) == other ? 0 : 1);
	}
	int __nonzero__() {
		return (IsNull(*self) ? 0 : 1);
	}
	#endif
}


// actual instruments

%{
using QuantLib::Instruments::Stock;

InstrumentHandle NewStock(String isinCode = "", String description = "") {
	return InstrumentHandle(new Stock(isinCode,description));
}
%}

%name(Stock) InstrumentHandle NewStock(String isinCode = "", 
    String description = "");


#endif
