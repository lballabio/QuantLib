
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_currency_h
#define quantlib_currency_h

#include "qldefines.h"
#include "calendar.h"
#include "date.h"
#include "handle.h"
#include <string>

QL_BEGIN_NAMESPACE(QuantLib)

class Currency {
  public:
	virtual std::string name() const = 0;	// used for output and comparisons - not for switch-on-type coding!
	// settlement conventions
	virtual Handle<Calendar> settlementCalendar() const = 0;
	virtual int settlementDays() const = 0;
	Date settlementDate(const Date&) const;
	// conventions for deposits or any other rates can be added
};

// comparison based on name

QL_DECLARE_TEMPLATE_SPECIALIZATION(bool operator==(const Handle<Currency>&, const Handle<Currency>&))
QL_DECLARE_TEMPLATE_SPECIALIZATION(bool operator!=(const Handle<Currency>&, const Handle<Currency>&))


// inline definitions

inline Date Currency::settlementDate(const Date& d) const {
	return settlementCalendar()->advance(d,settlementDays());
}


QL_DEFINE_TEMPLATE_SPECIALIZATION
inline bool operator==(const Handle<Currency>& c1, const Handle<Currency>& c2) {
	return (c1->name() == c2->name());
}

QL_DEFINE_TEMPLATE_SPECIALIZATION
inline bool operator!=(const Handle<Currency>& c1, const Handle<Currency>& c2) {
	return (c1->name() != c2->name());
}


QL_END_NAMESPACE(QuantLib)


#endif
