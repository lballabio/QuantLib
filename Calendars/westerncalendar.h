
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_western_calendar_h
#define quantlib_western_calendar_h

#include "qldefines.h"
#include "calendar.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Calendars)

class WesternCalendar : public Calendar {
  public:
	WesternCalendar() {}
  protected:
	static const Day easterMonday[];			// relative to first day of year
};

QL_END_NAMESPACE(Calendars)

QL_END_NAMESPACE(QuantLib)


#endif
