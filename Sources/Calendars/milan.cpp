
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "milan.h"

namespace QuantLib {

	namespace Calendars {
	
		bool Milan::isBusinessDay(const Date& date) const {
			Weekday w = date.dayOfWeek();
			Day d = date.dayOfMonth(), dd = date.dayOfYear();
			Month m = date.month();
			Year y = date.year();
			if ((w == Saturday || w == Sunday)
				// New Year's Day
				|| (d == 1 && m == January)
				// Epiphany
				|| (d == 6 && m == January)
				// Easter Monday
				|| (dd == easterMonday[y-1900])
				// Liberation Day
				|| (d == 25 && m == April)
				// Labour Day
				|| (d == 1 && m == May)
				// Assumption
				|| (d == 15 && m == August)
				// All Saints' Day
				|| (d == 1 && m == November)
				// Immaculate Conception
				|| (d == 8 && m == December)
				// Christmas
				|| (d == 25 && m == December)
				// St. Stephen
				|| (d == 26 && m == December)
				// December 31st, 1999 only
				|| (d == 31 && m == December && y == 1999))
					return false;
			return true;
		}
	
	}

}

