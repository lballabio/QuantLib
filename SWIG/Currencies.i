
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

#ifndef quantlib_currencies_i
#define quantlib_currencies_i

%module Currencies

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
%include Calendars.i
%include String.i

%{
using QuantLib::Currency;
using QuantLib::Handle;
typedef Handle<Currency> CurrencyHandle;
%}

// export Handle<Currency>
%name(Currency) class CurrencyHandle {
  public:
	// no constructor - forbid explicit construction
	~CurrencyHandle();
};

// replicate the Currency interface
%addmethods CurrencyHandle {
	CalendarHandle settlementCalendar() {
		return (*self)->settlementCalendar();
	}
	int settlementDays() {
		return (*self)->settlementDays();
	}
	Date settlementDate(const Date& d) {
		return (*self)->settlementDate(d);
	}
	#if defined (SWIGPYTHON)
	String __str__() {
		return (*self)->name()+" currency";
	}
	String __repr__() {
		return "<"+(*self)->name()+" currency>";
	}
	int __cmp__(const CurrencyHandle& other) {
		return ((*self) == other ? 0 : 1);
	}
	int __nonzero__() {
		return (IsNull(*self) ? 0 : 1);
	}
	#endif
}

// actual currencies

%{
using QuantLib::Currencies::EUR;
using QuantLib::Currencies::USD;
using QuantLib::Currencies::GBP;
using QuantLib::Currencies::DEM;
using QuantLib::Currencies::ITL;
using QuantLib::Currencies::AUD;
using QuantLib::Currencies::CAD;
using QuantLib::Currencies::CHF;
using QuantLib::Currencies::DKK;
using QuantLib::Currencies::JPY;
using QuantLib::Currencies::SEK;

CurrencyHandle NewEUR()		{ return CurrencyHandle(new EUR); }
CurrencyHandle NewUSD()		{ return CurrencyHandle(new USD); }
CurrencyHandle NewGBP()		{ return CurrencyHandle(new GBP); }
CurrencyHandle NewDEM()		{ return CurrencyHandle(new DEM); }
CurrencyHandle NewITL()		{ return CurrencyHandle(new ITL); }
CurrencyHandle NewAUD()		{ return CurrencyHandle(new AUD); }
CurrencyHandle NewCAD()		{ return CurrencyHandle(new CAD); }
CurrencyHandle NewCHF()		{ return CurrencyHandle(new CHF); }
CurrencyHandle NewDKK()		{ return CurrencyHandle(new DKK); }
CurrencyHandle NewJPY()		{ return CurrencyHandle(new JPY); }
CurrencyHandle NewSEK()		{ return CurrencyHandle(new SEK); }
%}

%name(EUR)	CurrencyHandle NewEUR();
%name(USD)	CurrencyHandle NewUSD();
%name(GBP)	CurrencyHandle NewGBP();
%name(DEM)	CurrencyHandle NewDEM();
%name(ITL)	CurrencyHandle NewITL();
%name(AUD)	CurrencyHandle NewAUD();
%name(CAD)	CurrencyHandle NewCAD();
%name(CHF)	CurrencyHandle NewCHF();
%name(DKK)	CurrencyHandle NewDKK();
%name(JPY)	CurrencyHandle NewJPY();
%name(SEK)	CurrencyHandle NewSEK();

// typemap Python list of currency handles to std::vector<Handle<Currency> >

%{
typedef std::vector<Handle<Currency> > CurrencyHandleVector;
%}

%typemap(python,in) CurrencyHandleVector, const CurrencyHandleVector & {
	if (PyTuple_Check($source)) {
		int size = PyTuple_Size($source);
		$target = new std::vector<CurrencyHandle>(size);
		for (int i=0; i<size; i++) {
			CurrencyHandle* d;
			PyObject* o = PyTuple_GetItem($source,i);
			if ((SWIG_ConvertPtr(o,(void **) &d,
			  (swig_type_info *)SWIG_TypeQuery("CurrencyHandle *"),1)) != -1) {
				(*$target)[i] = *d;
			} else {
				PyErr_SetString(PyExc_TypeError,
				  "tuple must contain currencies");
				delete $target;
				return NULL;
			}
		}
	} else if (PyList_Check($source)) {
		int size = PyList_Size($source);
		$target = new std::vector<CurrencyHandle>(size);
		for (int i=0; i<size; i++) {
			CurrencyHandle* d;
			PyObject* o = PyList_GetItem($source,i);
			if ((SWIG_ConvertPtr(o,(void **) &d,
			  (swig_type_info *)SWIG_TypeQuery("CurrencyHandle *"),1)) != -1) {
				(*$target)[i] = *d;
			} else {
				PyErr_SetString(PyExc_TypeError,"list must contain currencies");
				delete $target;
				return NULL;
			}
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a sequence");
		return NULL;
	}
};

%typemap(python,freearg) CurrencyHandleVector, const CurrencyHandleVector & {
	delete $source;
};


#endif
