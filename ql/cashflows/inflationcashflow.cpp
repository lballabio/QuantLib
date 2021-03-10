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

#include <ql/cashflows/inflationcashflow.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>

namespace QuantLib {
    ZeroInflationCashFlow::ZeroInflationCashFlow(Real notional,
                                                 const ext::shared_ptr<ZeroInflationIndex>& index,
                                                 CPI::InterpolationType observationInterpolation,
                                                 const Date& baseDate,
                                                 const Date& fixingDate,
                                                 const Date& paymentDate,
                                                 bool growthOnly)
    : IndexedCashFlow(notional, index, baseDate, fixingDate, paymentDate, growthOnly),
      zeroInflationIndex_(index), observationInterpolation_(observationInterpolation) {}

    Real ZeroInflationCashFlow::amount() const {
        // work out what it should be
        auto baseDatePeriod = inflationPeriod(baseDate(), zeroInflationIndex()->frequency());
        auto fixingDatePeriod = inflationPeriod(fixingDate(), zeroInflationIndex()->frequency());

        Real fixingFront{}, fixingEnd{};

        if (observationInterpolation_ == CPI::AsIndex) {
            fixingFront = zeroInflationIndex_->fixing(baseDate());
            fixingEnd = zeroInflationIndex_->fixing(fixingDate());
        } else if (observationInterpolation_ == CPI::Linear) {
            auto getInterpolatedFixings = [this](const std::pair<Date, Date>& period,
                                                 const Date& date) -> Real {
                auto oneDay = Period(1, Days);
                auto indexStart = zeroInflationIndex_->fixing(period.first);
                auto indexEnd = zeroInflationIndex_->fixing(period.second + oneDay);

                return indexStart + (indexEnd - indexStart) * (date - period.first) /
                                        (Real)((period.second + oneDay) - period.first);
            };

            fixingFront = getInterpolatedFixings(baseDatePeriod, baseDate());
            fixingEnd = getInterpolatedFixings(fixingDatePeriod, fixingDate());
        } else if (observationInterpolation_ == CPI::Flat) {
            fixingFront = zeroInflationIndex_->fixing(baseDatePeriod.first);
            fixingEnd = zeroInflationIndex_->fixing(fixingDatePeriod.first);
        } else {
            // We should end up here...
            QL_FAIL("Unknown ZeroInflationInterpolationType.");
        }

        if (growthOnly())
            return notional() * (fixingEnd / fixingFront - 1.0);
        else
            return notional() * (fixingEnd / fixingFront);
    }
}
