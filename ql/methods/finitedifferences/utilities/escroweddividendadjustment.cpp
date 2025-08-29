/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file escroweddividendadjustment.cpp
*/
#include <ql/methods/finitedifferences/utilities/escroweddividendadjustment.hpp>

namespace QuantLib {

    EscrowedDividendAdjustment::EscrowedDividendAdjustment(
        DividendSchedule dividendSchedule,
        Handle<YieldTermStructure> rTS,
        Handle<YieldTermStructure> qTS,
        std::function<Real(Date)> toTime,
        Time maturity)
    : dividendSchedule_(std::move(dividendSchedule)),
      rTS_(std::move(rTS)), qTS_(std::move(qTS)),
      toTime_(std::move(toTime)), maturity_(maturity) {}


    Real EscrowedDividendAdjustment::dividendAdjustment(Time t) const {

        Real divAdj = 0.0;
        for (auto const& dividend: dividendSchedule_) {
            const Time divTime = toTime_(dividend->date());

            if (divTime >= t && t <= maturity_)
                divAdj -= dividend->amount()
                    * rTS_->discount(divTime) / rTS_->discount(t)
                    * qTS_->discount(t) / qTS_->discount(divTime);
        }

        return divAdj;
    }

    const Handle<YieldTermStructure>&
    EscrowedDividendAdjustment::riskFreeRate() const {
        return rTS_;
    }

    const Handle<YieldTermStructure>&
    EscrowedDividendAdjustment::dividendYield() const {
        return qTS_;
    }
}
