
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_day_counter_h
#define quantlib_day_counter_h

#include "qldefines.h"
#include "date.h"
#include "handle.h"

QL_BEGIN_NAMESPACE(QuantLib)

class DayCounter {
  public:
	virtual std::string name() const = 0;	// only for output and comparisons - do not use for switch-on-type code!
	virtual int dayCount(const Date&, const Date&) const = 0;
	virtual Time yearFraction(const Date&, const Date&, 
	  const Date& refPeriodStart = Date(), const Date& refPeriodEnd = Date()) const = 0;
};

// comparison based on name

QL_DECLARE_TEMPLATE_SPECIALIZATION(bool operator==(const Handle<DayCounter>&, const Handle<DayCounter>&))
QL_DECLARE_TEMPLATE_SPECIALIZATION(bool operator!=(const Handle<DayCounter>&, const Handle<DayCounter>&))


// inline definitions

QL_DEFINE_TEMPLATE_SPECIALIZATION
inline bool operator==(const Handle<DayCounter>& h1, const Handle<DayCounter>& h2) {
	return (h1->name() == h2->name());
}

QL_DEFINE_TEMPLATE_SPECIALIZATION
inline bool operator!=(const Handle<DayCounter>& h1, const Handle<DayCounter>& h2) {
	return (h1->name() != h2->name());
}


QL_END_NAMESPACE(QuantLib)


#endif
