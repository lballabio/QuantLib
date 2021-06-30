/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Ralf Konrad Eckel

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

#include <ql/cashflows/zeroinflationcashflow.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/time/calendars/nullcalendar.hpp>

namespace QuantLib {

    ZeroInflationCashFlow::ZeroInflationCashFlow(Real notional,
                                                 const ext::shared_ptr<ZeroInflationIndex>& index,
                                                 CPI::InterpolationType observationInterpolation,
                                                 const Date& startDate,
                                                 const Date& endDate,
                                                 const Period& observationLag,
                                                 const Date& paymentDate,
                                                 bool growthOnly)
    : ZeroInflationCashFlow(notional, index, observationInterpolation,
                            startDate, endDate, observationLag,
                            NullCalendar(), Unadjusted, paymentDate, growthOnly) {}

    ZeroInflationCashFlow::ZeroInflationCashFlow(Real notional,
                                                 const ext::shared_ptr<ZeroInflationIndex>& index,
                                                 CPI::InterpolationType observationInterpolation,
                                                 const Date& startDate,
                                                 const Date& endDate,
                                                 const Period& observationLag,
                                                 const Calendar& calendar,
                                                 BusinessDayConvention convention,
                                                 const Date& paymentDate,
                                                 bool growthOnly)
    : IndexedCashFlow(notional, index,
                      calendar.adjust(startDate - observationLag, convention),
                      calendar.adjust(endDate - observationLag, convention),
                      paymentDate, growthOnly),
      zeroInflationIndex_(index), observationInterpolation_(observationInterpolation),
      startDate_(startDate), endDate_(endDate), observationLag_(observationLag) {}

    Real ZeroInflationCashFlow::amount() const {

        auto baseDatePeriod = inflationPeriod(baseDate(), zeroInflationIndex()->frequency());
        auto fixingDatePeriod = inflationPeriod(fixingDate(), zeroInflationIndex()->frequency());

        Real I0, I1;

        if (observationInterpolation_ == CPI::AsIndex) {
            I0 = zeroInflationIndex_->fixing(baseDate());
            I1 = zeroInflationIndex_->fixing(fixingDate());
        } else if (observationInterpolation_ == CPI::Linear) {
            auto getInterpolatedFixing = [this](const std::pair<Date, Date>& fixingPeriod,
                                                 const Date& date) -> Real {
                auto oneDay = Period(1, Days);
                auto startIndex = zeroInflationIndex_->fixing(fixingPeriod.first);
                auto endIndex = zeroInflationIndex_->fixing(fixingPeriod.second + oneDay);

                auto interpolationPeriod = inflationPeriod(date, zeroInflationIndex()->frequency());

                return startIndex + (endIndex - startIndex) * (date - interpolationPeriod.first) /
                    (Real)((interpolationPeriod.second + oneDay) - interpolationPeriod.first);
            };

            I0 = getInterpolatedFixing(baseDatePeriod, startDate_);
            I1 = getInterpolatedFixing(fixingDatePeriod, endDate_);
        } else if (observationInterpolation_ == CPI::Flat) {
            I0 = zeroInflationIndex_->fixing(baseDatePeriod.first);
            I1 = zeroInflationIndex_->fixing(fixingDatePeriod.first);
        } else {
            // We should not end up here...
            QL_FAIL("Unknown ZeroInflationInterpolationType.");
        }

        if (growthOnly())
            return notional() * (I1 / I0 - 1.0);
        else
            return notional() * (I1 / I0);
    }

    void ZeroInflationCashFlow::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<ZeroInflationCashFlow>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            IndexedCashFlow::accept(v);
    }

}
