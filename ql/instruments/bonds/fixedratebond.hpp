/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2004 M-Dimension Consulting Inc.
 Copyright (C) 2005 StatPro Italia srl
 Copyright (C) 2007, 2008, 2010 Ferdinando Ametrano
 Copyright (C) 2009 Piter Dias

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

/*! \file fixedratebond.hpp
    \brief fixed-rate bond
*/

#ifndef quantlib_fixed_rate_bond_hpp
#define quantlib_fixed_rate_bond_hpp

#include <ql/instruments/bond.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/interestrate.hpp>

namespace QuantLib {

    class Schedule;

    //! fixed-rate bond
    /*! \ingroup instruments

        \test calculations are tested by checking results against
              cached values.
    */
    class FixedRateBond : public Bond {
      public:
        //! simple annual compounding coupon rates
        FixedRateBond(Natural settlementDays,
                      Real faceAmount,
                      Schedule schedule,
                      const std::vector<Rate>& coupons,
                      const DayCounter& accrualDayCounter,
                      BusinessDayConvention paymentConvention = Following,
                      Real redemption = 100.0,
                      const Date& issueDate = Date(),
                      const Calendar& paymentCalendar = Calendar(),
                      const Period& exCouponPeriod = Period(),
                      const Calendar& exCouponCalendar = Calendar(),
                      BusinessDayConvention exCouponConvention = Unadjusted,
                      bool exCouponEndOfMonth = false,
                      const DayCounter& firstPeriodDayCounter = DayCounter());

        Frequency frequency() const { return frequency_; }
        const DayCounter& dayCounter() const { return dayCounter_; }
        const DayCounter& firstPeriodDayCounter() const { return firstPeriodDayCounter_; }
      protected:
        Frequency frequency_;
        DayCounter dayCounter_;
        DayCounter firstPeriodDayCounter_;
    };

}

#endif
