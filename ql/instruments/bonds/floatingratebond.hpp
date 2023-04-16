/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Chiara Fornarola

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

/*! \file floatingratebond.hpp
    \brief floating-rate bond
*/

#ifndef quantlib_floating_rate_bond_hpp
#define quantlib_floating_rate_bond_hpp

#include <ql/instruments/bond.hpp>
#include <ql/time/dategenerationrule.hpp>

namespace QuantLib {

    class Schedule;
    class IborIndex;

    //! floating-rate bond (possibly capped and/or floored)
    /*! \ingroup instruments

        \test calculations are tested by checking results against
              cached values.
    */
    class FloatingRateBond : public Bond {
      public:
        FloatingRateBond(Natural settlementDays,
                         Real faceAmount,
                         const Schedule& schedule,
                         const std::shared_ptr<IborIndex>& iborIndex,
                         const DayCounter& accrualDayCounter,
                         BusinessDayConvention paymentConvention = Following,
                         Natural fixingDays = Null<Natural>(),
                         const std::vector<Real>& gearings = { 1.0 },
                         const std::vector<Spread>& spreads = { 0.0 },
                         const std::vector<Rate>& caps = {},
                         const std::vector<Rate>& floors = {},
                         bool inArrears = false,
                         Real redemption = 100.0,
                         const Date& issueDate = Date(),
                         const Period& exCouponPeriod = Period(),
                         const Calendar& exCouponCalendar = Calendar(),
                         BusinessDayConvention exCouponConvention = Unadjusted,
                         bool exCouponEndOfMonth = false);
        
        /*! \deprecated Use the other constructor.
                        Deprecated in version 1.28.
        */
        QL_DEPRECATED
        FloatingRateBond(Natural settlementDays,
                         Real faceAmount,
                         const Date& startDate,
                         const Date& maturityDate,
                         Frequency couponFrequency,
                         const Calendar& calendar,
                         const std::shared_ptr<IborIndex>& iborIndex,
                         const DayCounter& accrualDayCounter,
                         BusinessDayConvention accrualConvention = Following,
                         BusinessDayConvention paymentConvention = Following,
                         Natural fixingDays = Null<Natural>(),
                         const std::vector<Real>& gearings = { 1.0 },
                         const std::vector<Spread>& spreads = { 0.0 },
                         const std::vector<Rate>& caps = {},
                         const std::vector<Rate>& floors = {},
                         bool inArrears = false,
                         Real redemption = 100.0,
                         const Date& issueDate = Date(),
                         const Date& stubDate = Date(),
                         DateGeneration::Rule rule = DateGeneration::Backward,
                         bool endOfMonth = false,
                         const Period& exCouponPeriod = Period(),
                         const Calendar& exCouponCalendar = Calendar(),
                         BusinessDayConvention exCouponConvention = Unadjusted,
                         bool exCouponEndOfMonth = false);
    };

}

#endif
