
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_instruments_i
#define quantlib_instruments_i

%module Instruments

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%include Financial.i
%include TermStructures.i

%{
	#include "instrument.h"
	#include <cstdlib>

	QL_USING(QuantLib,Instrument)
	QL_USING(QuantLib,Handle)

	typedef Handle<Instrument> InstrumentHandle;
%}

%{
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
	// abstract class - forbid explicit construction
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
	char* isinCode() {
		static char temp[256];
		strcpy(temp,(*self)->isinCode().c_str());
		return temp;
	}
	char* description() {
		static char temp[256];
		strcpy(temp,(*self)->description().c_str());
		return temp;
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
	char* __str__() {
		static char temp[256];
		strcpy(temp,Representation(*self).c_str());
		return temp;
	}
	char* __repr__() {
		static char temp[256];
		std::string s = "<Handle<Instrument>: "+Representation(*self)+">";
		strcpy(temp,s.c_str());
		return temp;
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
#include "stock.h"
QL_USING(QuantLib::Instruments,Stock)

InstrumentHandle NewStock(char* isinCode, char* description) {
	return InstrumentHandle(new Stock(isinCode,description));
}
%}

%name(Stock) InstrumentHandle NewStock(char* isinCode, char* description);


#endif
