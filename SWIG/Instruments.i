
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
   Revision 1.18  2001/04/06 18:46:21  nando
   changed Authors, Contributors, Licence and copyright header

   Revision 1.17  2001/03/27 13:44:37  lballabio
   Removed default string arguments (crashed the thing when used)

   Revision 1.16  2001/03/12 17:35:11  lballabio
   Removed global IsNull function - could have caused very vicious loops

   Revision 1.15  2001/03/12 12:59:01  marmar
   __str__ now represents the object while __repr__ is unchanged

   Revision 1.14  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_instruments_i
#define quantlib_instruments_i

%include Financial.i
%include TermStructures.i
%include String.i

%{
#include <cstdlib>
using QuantLib::Instrument;
using QuantLib::Handle;
typedef Handle<Instrument> InstrumentHandle;
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
	    if (self->isNull())
	        return "Null instrument";
    	std::string isin = (*self)->isinCode();
    	if (isin == "")
    		isin = "unknown";
    	std::string desc = (*self)->description();
    	if (desc == "")
    		desc = "no description available";
    	return ("Instrument: "+isin+" ("+desc+")");
	}
	int __cmp__(const InstrumentHandle& other) {
		return ((*self) == other ? 0 : 1);
	}
	int __nonzero__() {
		return (self->isNull() ? 0 : 1);
	}
	#endif
}


// actual instruments

%{
using QuantLib::Instruments::Stock;

InstrumentHandle NewStock(String isinCode, String description) {
	return InstrumentHandle(new Stock(isinCode,description));
}
%}

%name(Stock) InstrumentHandle NewStock(String isinCode, String description);


#endif
