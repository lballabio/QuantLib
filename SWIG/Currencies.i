
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

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%include Date.i
%include Calendars.i

%{
#include "currency.h"
#include "eur.h"
#include "usd.h"
#include "gbp.h"
#include "dem.h"
#include "itl.h"

using QuantLib::Currency;
using QuantLib::Handle;
typedef Handle<Currency> CurrencyHandle;
%}

// export Handle<Currency>
%name(Currency) class CurrencyHandle {
  public:
	// abstract class - forbid explicit construction
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
	char* __str__() {
		static char temp[256];
		sprintf(temp,"%s currency",(*self)->name().c_str());
		return temp;
	}
	char* __repr__() {
		static char temp[256];
		sprintf(temp,"<%s currency>",(*self)->name().c_str());
		return temp;
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

CurrencyHandle NewEUR()		{ return CurrencyHandle(new EUR); }
CurrencyHandle NewUSD()		{ return CurrencyHandle(new USD); }
CurrencyHandle NewGBP()		{ return CurrencyHandle(new GBP); }
CurrencyHandle NewDEM()		{ return CurrencyHandle(new DEM); }
CurrencyHandle NewITL()		{ return CurrencyHandle(new ITL); }
%}

%name(EUR)	CurrencyHandle NewEUR();
%name(USD)	CurrencyHandle NewUSD();
%name(GBP)	CurrencyHandle NewGBP();
%name(DEM)	CurrencyHandle NewDEM();
%name(ITL)	CurrencyHandle NewITL();


#endif
