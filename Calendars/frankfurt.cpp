
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "frankfurt.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Calendars)

bool Frankfurt::isBusinessDay(const Date& date) const {
	Weekday w = date.dayOfWeek();
	Day d = date.dayOfMonth(), dd = date.dayOfYear();
	Month m = date.month();
	Year y = date.year()-1900;
	if ((w == Saturday || w == Sunday)
		// New Year's Day 
		|| (d == 1 && m == January)
		// Good Friday
		|| (dd == easterMonday[y]-3)
		// Easter Monday
		|| (dd == easterMonday[y])
		// Ascension Thursday
		|| (dd == easterMonday[y]+38)
		// Whit Monday
		|| (dd == easterMonday[y]+49)
		// Corpus Christi
		|| (dd == easterMonday[y]+59)
		// Labour Day
		|| (d == 1 && m == May)
		// National Day
		|| (d == 3 && m == October)
		// Christmas Eve
		|| (d == 24 && m == December)
		// Christmas
		|| (d == 25 && m == December)
		// Boxing Day
		|| (d == 26 && m == December)
		// New Year's Eve 
		|| (d == 31 && m == December))
			return false;
	return true;
}

QL_END_NAMESPACE(Calendars)

QL_END_NAMESPACE(QuantLib)

