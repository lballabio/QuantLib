/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Klaus Spanderen

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

/*! \file escroweddividendadjustment.hpp
*/

#ifndef quantlib_escrowed_dividend_adjustment_hpp
#define quantlib_escrowed_dividend_adjustment_hpp

#include <ql/handle.hpp>
#include <ql/functional.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/instruments/dividendschedule.hpp>

namespace QuantLib {

    class EscrowedDividendAdjustment {
      public:
        EscrowedDividendAdjustment(
            DividendSchedule dividendSchedule,
            Handle<YieldTermStructure> rTS,
            Handle<YieldTermStructure> qTS,
            std::function<Real(Date)> toTime,
            Time maturity);

        Real dividendAdjustment(Time t) const;

        const Handle<YieldTermStructure>& riskFreeRate() const;
        const Handle<YieldTermStructure>& dividendYield() const;

      private:
        const DividendSchedule dividendSchedule_;
        const Handle<YieldTermStructure> rTS_, qTS_;
        const std::function<Real(Date)> toTime_;
        const Time maturity_;
    };
}

#endif
