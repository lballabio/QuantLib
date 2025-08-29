/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Simon Ibbotson

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

/*! \file amortizingfloatingratebond.hpp
    \brief amortizing floating-rate bond
*/

#ifndef quantlib_amortizing_floating_rate_bond_hpp
#define quantlib_amortizing_floating_rate_bond_hpp

#include <ql/instruments/bond.hpp>

namespace QuantLib {

    class Schedule;
    class IborIndex;

    //! amortizing floating-rate bond (possibly capped and/or floored)
    class AmortizingFloatingRateBond : public Bond {
      public:
        AmortizingFloatingRateBond(Natural settlementDays,
                                   const std::vector<Real>& notional,
                                   Schedule schedule,
                                   const ext::shared_ptr<IborIndex>& index,
                                   const DayCounter& accrualDayCounter,
                                   BusinessDayConvention paymentConvention = Following,
                                   Natural fixingDays = Null<Natural>(),
                                   const std::vector<Real>& gearings = { 1.0 },
                                   const std::vector<Spread>& spreads = { 0.0 },
                                   const std::vector<Rate>& caps = {},
                                   const std::vector<Rate>& floors = {},
                                   bool inArrears = false,
                                   const Date& issueDate = Date(),
                                   const Period& exCouponPeriod = Period(),
                                   const Calendar& exCouponCalendar = Calendar(),
                                   BusinessDayConvention exCouponConvention = Unadjusted,
                                   bool exCouponEndOfMonth = false,
                                   const std::vector<Real>& redemptions = { 100.0 },
                                   Integer paymentLag = 0);
    };

}

#endif
