
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "target.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Calendars)

bool TARGET::isBusinessDay(const Date& date) const {
	Weekday w = date.dayOfWeek();
	Day d = date.dayOfMonth(), dd = date.dayOfYear();
	Month m = date.month();
	Year y = date.year();
	if ((w == Saturday || w == Sunday)
		// New Year's Day
		|| (d == 1  && m == January)
		// Good Friday
		|| (dd == easterMonday[y-1900]-3)
		// Easter Monday
		|| (dd == easterMonday[y-1900])
		// Labour Day
		|| (d == 1  && m == May)
		// Christmas
		|| (d == 25 && m == December)
		// Day of Goodwill
		|| (d == 26 && m == December)
		// December 31st, 1998 and 1999 only
		|| (d == 31 && m == December && (y == 1998 || y == 1999)))
			return false;
	return true;
}

QL_END_NAMESPACE(Calendars)

QL_END_NAMESPACE(QuantLib)

