
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_GBP_h
#define quantlib_GBP_h

#include "qldefines.h"
#include "currency.h"
#include "london.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Currencies)

class GBP : public Currency {
  public:
	GBP() {}
	std::string name() const { return std::string("GBP"); }
	Handle<Calendar> settlementCalendar() const { 
		return Handle<Calendar>(new QL_ADD_NAMESPACE(Calendars,London)); }
	int settlementDays() const { return 0; }
};

QL_END_NAMESPACE(Currencies)

QL_END_NAMESPACE(QuantLib)


#endif
