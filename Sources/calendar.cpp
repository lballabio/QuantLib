
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "calendar.h"

QL_BEGIN_NAMESPACE(QuantLib)

Date Calendar::roll(RollingConvention convention, const Date& d) const {
	Date d1 = d;
	switch (convention) {
	  case Following:
	  case ModifiedFollowing:
		while (isHoliday(d1))
			d1++;
		if (convention == ModifiedFollowing && d1.month() != d.month())
			return roll(Preceding,d);
		break;
	  case Preceding:
	  case ModifiedPreceding:
		while (isHoliday(d1))
			d1--;
		if (convention == ModifiedPreceding && d1.month() != d.month())
			return roll(Following,d);
		break;
	  default:
		throw IllegalArgumentError("Unknown rolling convention");
	}
	return d1;
}

Date Calendar::advance(const Date& d, int businessDays) const {
	Date d1 = d;
	if (businessDays >= 0) {
		while (businessDays >= 0) {
			while (isHoliday(d1))
				d1++;
			businessDays--;
		}
	} else {
		while (businessDays <= 0) {
			while(isHoliday(d1))
				d1--;
			businessDays--;
		}
	}
	return d1;
}


QL_END_NAMESPACE(QuantLib)
