
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_london_calendar_h
#define quantlib_london_calendar_h

#include "qldefines.h"
#include "westerncalendar.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Calendars)

class London : public WesternCalendar {
  public:
	London() {}
	std::string name() const { return "London"; }
	bool isBusinessDay(const Date&) const;
};

QL_END_NAMESPACE(Calendars)

QL_END_NAMESPACE(QuantLib)


#endif
