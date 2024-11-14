/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <ql/indexes/ibor/custom.hpp>

namespace QuantLib {

    CustomIborIndex::CustomIborIndex(const std::string& familyName,
                                     const Period& tenor,
                                     Natural settlementDays,
                                     const Currency& currency,
                                     const Calendar& fixingCalendar,
                                     const Calendar& valueCalendar,
                                     const Calendar& maturityCalendar,
                                     BusinessDayConvention convention,
                                     bool endOfMonth,
                                     const DayCounter& dayCounter,
                                     const Handle<YieldTermStructure>& h)
    : IborIndex(familyName, tenor, settlementDays, currency, fixingCalendar,
                convention, endOfMonth, dayCounter, h),
      valueCalendar_(valueCalendar), maturityCalendar_(maturityCalendar) {}

    Date CustomIborIndex::fixingDate(const Date& valueDate) const {
        Date fixingDate = valueCalendar_.advance(valueDate,
            -static_cast<Integer>(fixingDays_), Days);
        return fixingCalendar().adjust(fixingDate, Preceding);
    }

    Date CustomIborIndex::valueDate(const Date& fixingDate) const {

        QL_REQUIRE(isValidFixingDate(fixingDate),
                   "Fixing date " << fixingDate << " is not valid");

        Date d = valueCalendar_.advance(fixingDate, fixingDays_, Days);
        return maturityCalendar_.adjust(d);
    }

    Date CustomIborIndex::maturityDate(const Date& valueDate) const {
        return maturityCalendar_.advance(valueDate, tenor_, convention_,
                                         endOfMonth_);
    }

    ext::shared_ptr<IborIndex> CustomIborIndex::clone(
            const Handle<YieldTermStructure>& h) const {
        return ext::make_shared<CustomIborIndex>(
            familyName_, tenor_, fixingDays_, currency_, fixingCalendar(),
            valueCalendar_, maturityCalendar_, convention_, endOfMonth_,
            dayCounter_, h);
    }

}
