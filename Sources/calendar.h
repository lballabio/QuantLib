
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_calendar_h
#define quantlib_calendar_h

#include "qldefines.h"
#include "qlerrors.h"
#include "date.h"
#include "handle.h"
#include <string>

QL_BEGIN_NAMESPACE(QuantLib)

enum RollingConvention { Following, ModifiedFollowing, Preceding, ModifiedPreceding };

// calendar

class Calendar {
  public:
	Calendar() {}
	// interface for derived calendars
	virtual std::string name() const = 0;
	virtual bool isBusinessDay(const Date&) const = 0;
	bool isHoliday(const Date& d) const { return !isBusinessDay(d); }
	Date roll(RollingConvention, const Date&) const;
	Date advance(const Date&, int businessDays) const;
};

// comparison based on name

QL_DECLARE_TEMPLATE_SPECIALIZATION(bool operator==(const Handle<Calendar>&, const Handle<Calendar>&))
QL_DECLARE_TEMPLATE_SPECIALIZATION(bool operator!=(const Handle<Calendar>&, const Handle<Calendar>&))

// null calendar (no holidays, not even saturdays and sundays)

QL_BEGIN_NAMESPACE(Calendars)

class NullCalendar : public Calendar {
  public:
	NullCalendar() {}
	std::string name() const { return "None"; }
	bool isBusinessDay(const Date& d) const { d; return true; }
};

QL_END_NAMESPACE(Calendars)

// inline definitions

QL_DEFINE_TEMPLATE_SPECIALIZATION
inline bool operator==(const Handle<Calendar>& h1, const Handle<Calendar>& h2) {
	return (h1->name() == h2->name());
}

QL_DEFINE_TEMPLATE_SPECIALIZATION
inline bool operator!=(const Handle<Calendar>& h1, const Handle<Calendar>& h2) {
	return (h1->name() != h2->name());
}


QL_END_NAMESPACE(QuantLib)


#endif
