
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_currencies_i
#define quantlib_currencies_i

%module Currencies

%include Date.i
%include Calendars.i

%{
#include "currency.h"
#include "eur.h"
#include "usd.h"
#include "gbp.h"
#include "dem.h"
#include "itl.h"

QL_USING(QuantLib,Currency)
QL_USING(QuantLib,Handle)
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
QL_USING(QuantLib::Currencies,EUR)
QL_USING(QuantLib::Currencies,USD)
QL_USING(QuantLib::Currencies,GBP)
QL_USING(QuantLib::Currencies,DEM)
QL_USING(QuantLib::Currencies,ITL)

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
