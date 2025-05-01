/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 StatPro Italia srl
 Copyright (C) 2009 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/indexes/iborindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/schedule.hpp>
#include <utility>
#include <numeric>

namespace QuantLib {

    IborIndex::IborIndex(const std::string& familyName,
                         const Period& tenor,
                         Natural settlementDays,
                         const Currency& currency,
                         const Calendar& fixingCalendar,
                         BusinessDayConvention convention,
                         bool endOfMonth,
                         const DayCounter& dayCounter,
                         Handle<YieldTermStructure> h)
    : InterestRateIndex(familyName, tenor, settlementDays, currency, fixingCalendar, dayCounter),
      convention_(convention), termStructure_(std::move(h)), endOfMonth_(endOfMonth) {
        registerWith(termStructure_);
    }

    Rate IborIndex::forecastFixing(const Date& fixingDate) const {
        Date d1 = valueDate(fixingDate);
        Date d2 = maturityDate(d1);
        Time t = dayCounter_.yearFraction(d1, d2);
        QL_REQUIRE(t>0.0,
                   "\n cannot calculate forward rate between " <<
                   d1 << " and " << d2 <<
                   ":\n non positive time (" << t <<
                   ") using " << dayCounter_.name() << " daycounter");
        return forecastFixing(d1, d2, t);
    }

    Date IborIndex::maturityDate(const Date& valueDate) const {
        return fixingCalendar().advance(valueDate,
                                        tenor_,
                                        convention_,
                                        endOfMonth_);
    }

    ext::shared_ptr<IborIndex> IborIndex::clone(
                               const Handle<YieldTermStructure>& h) const {
        return ext::make_shared<IborIndex>(
                                        familyName(),
                                                      tenor(),
                                                      fixingDays(),
                                                      currency(),
                                                      fixingCalendar(),
                                                      businessDayConvention(),
                                                      endOfMonth(),
                                                      dayCounter(),
                                                      h);
    }


    OvernightIndex::OvernightIndex(const std::string& familyName,
                                   Natural settlementDays,
                                   const Currency& curr,
                                   const Calendar& fixCal,
                                   const DayCounter& dc,
                                   const Handle<YieldTermStructure>& h)
   : IborIndex(familyName, 1*Days, settlementDays, curr,
               fixCal, Following, false, dc, h) {}

    void OvernightIndex::addFixing(const Date& fixingDate, Real fixing, bool forceOverwrite) {
        calculated_ = false;
        Index::addFixing(fixingDate, fixing, forceOverwrite);
    }

    void OvernightIndex::addFixings(const TimeSeries<Real>& t, bool forceOverwrite) {
        calculated_ = false;
        Index::addFixings(t, forceOverwrite);
    }

    Rate OvernightIndex::compoundedFixings(const Date& fromFixingDate, const Date& toFixingDate) {
        calculate();
        auto yearFraction = dayCounter_.yearFraction(fromFixingDate, toFixingDate);
        auto compIndexEnd = compoundIndex_[toFixingDate];
        auto compIndexStart = compoundIndex_[fromFixingDate];

        if (compIndexStart == Null<Real>() || compIndexEnd == Null<Real>())
            return Null<Real>();

        return (compoundIndex_[toFixingDate] / compoundIndex_[fromFixingDate] - 1) / yearFraction;  
    }

    Real OvernightIndex::compoundedFactor(const Date& fromFixingDate, const Date& toFixingDate) {
        calculate();
        auto compIndexEnd = compoundIndex_[toFixingDate];
        auto compIndexStart = compoundIndex_[fromFixingDate];

        if (compIndexStart == Null<Real>() || compIndexEnd == Null<Real>())
            return Null<Real>();

        return compIndexEnd / compIndexStart;
    }
 
    void OvernightIndex::performCalculations() const {
        auto getLastFixingDate = [](Calendar fixingCalendar, std::vector<Date>& fixingDates){
            Date lastFixingDay = fixingDates.front();
            std::for_each(fixingDates.begin() + 1, fixingDates.end(), [&](Date& fixingDate){
                if (fixingCalendar.advance(lastFixingDay, Period(1, Days)) == fixingDate)
                    lastFixingDay = fixingDate;
            });
            return lastFixingDay;
        };

        const auto& ts = Index::timeSeries();
        auto fixingDates = ts.dates();
        if (fixingDates.empty()) {
            compoundIndex_ = TimeSeries<Real>();
            return;
        }

        auto fixingCalendar = InterestRateIndex::fixingCalendar();
        auto lastFixingDate = getLastFixingDate(fixingCalendar, fixingDates);
        Schedule schedule = MakeSchedule()
                                .from(fixingDates.front())
                                .to(lastFixingDate)
                                .withTenor(Period(1, Days))
                                .withCalendar(fixingCalendar)
                                .withConvention(Following)
                                .withTerminationDateConvention(Following);
        std::vector<Real> compIndexValues(schedule.size());
        Real lastCompIndexValue = 1.000;
        compIndexValues[0] = lastCompIndexValue;

        std::transform(schedule.begin(), schedule.end() - 1, schedule.begin() + 1, compIndexValues.begin() + 1,
            [&](const Date& d1, const Date& d2) {
                lastCompIndexValue *= (1 + ts[d1] * dayCounter_.yearFraction(d1, d2));
                return lastCompIndexValue;
            });

        compoundIndex_ = TimeSeries<Real>(schedule.begin(), 
                                          schedule.end(), 
                                          compIndexValues.begin());
    }

    void OvernightIndex::update() {
        notifyObservers();
    }

    ext::shared_ptr<IborIndex> OvernightIndex::clone(
                               const Handle<YieldTermStructure>& h) const {
        return ext::shared_ptr<IborIndex>(
                                        new OvernightIndex(familyName(),
                                                           fixingDays(),
                                                           currency(),
                                                           fixingCalendar(),
                                                           dayCounter(),
                                                           h));
    }

}
