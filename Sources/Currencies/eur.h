
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_EUR_h
#define quantlib_EUR_h

#include "qldefines.h"
#include "currency.h"
#include "target.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Currencies)

class EUR : public Currency {
  public:
	EUR() {}
	std::string name() const { return std::string("EUR"); }
	Handle<Calendar> settlementCalendar() const {
		return Handle<Calendar>(new QL_ADD_NAMESPACE(Calendars,TARGET)); }
	int settlementDays() const { return 2; }
};

QL_END_NAMESPACE(Currencies)

QL_END_NAMESPACE(QuantLib)


#endif
