
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "london.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Calendars)

bool London::isBusinessDay(const Date& date) const {
	Weekday w = date.dayOfWeek();
	Day d = date.dayOfMonth(), dd = date.dayOfYear();
	Month m = date.month();
	Year y = date.year();
	if ((w == Saturday || w == Sunday)
		// New Year's Day (possibly moved to Monday)
		|| ((d == 1 || ((d == 2 || d == 3) && w == Monday)) && m == January)
		// Good Friday
		|| (dd == easterMonday[y-1900]-3)
		// Easter Monday
		|| (d == easterMonday[y-1900])
		// first Monday of May (Bank Holiday)
		|| (d <= 7 && w == Monday && m == May)
		// last Monday of May or August (Bank Holidays)
		|| (d >= 25 && w == Monday && (m == May || m == August))
		// Christmas (possibly moved to Monday or Tuesday)
		|| ((d == 25 || (d == 27 && (w == Monday || w == Tuesday))) && m == December)
		// Boxing Day (possibly moved to Monday or Tuesday)
		|| ((d == 26 || (d == 28 && (w == Monday || w == Tuesday))) && m == December)
		// December 31st, 1999 only
		|| (d == 31 && m == December && y == 1999))
			return false;
	return true;
}

QL_END_NAMESPACE(Calendars)

QL_END_NAMESPACE(QuantLib)

