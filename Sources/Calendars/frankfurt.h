
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_frankfurt_calendar_h
#define quantlib_frankfurt_calendar_h

#include "qldefines.h"
#include "westerncalendar.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Calendars)

class Frankfurt : public WesternCalendar {
  public:
	Frankfurt() {}
	std::string name() const { return "Frankfurt"; }
	bool isBusinessDay(const Date&) const;
};

QL_END_NAMESPACE(Calendars)

QL_END_NAMESPACE(QuantLib)


#endif
