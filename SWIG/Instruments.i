
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
    Revision 1.20  2001/05/03 10:26:07  lballabio
    Stock is now an actual shadow class

    Revision 1.19  2001/04/09 12:24:58  nando
    updated copyright notice header and improved CVS tags

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
    typedef Handle<Instrument> StockHandle;
%}

// actually they are the same class, but I'll fake inheritance...

%name(Stock) class StockHandle : public InstrumentHandle {
  public:
    // constructor redefined below
    ~StockHandle();
};

%addmethods StockHandle {
    StockHandle(String isinCode, String description) {
        return new StockHandle(new Stock(isinCode,description));
    }
}


#endif
