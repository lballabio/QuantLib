/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chiara Fornarola

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file cappedflooredcouponbond.hpp
    \brief reverse floater capped-floored coupon bond
*/

#ifndef quantlib_cappedfloored_coupon_bond_hpp
#define quantlib_cappedfloored_coupon_bond_hpp

#include <ql/Instruments/bond.hpp>
#include <ql/Indexes/iborindex.hpp>
#include <ql/PricingEngines/core.hpp>
#include <ql/capvolstructures.hpp>

namespace QuantLib {

    //! capped & floored coupon bond
    /*! \ingroup instruments

        \test calculations are tested by checking results against
              cached values.
    */
    class CappedFlooredCouponBond : public Bond {
      public:
          CappedFlooredCouponBond(
             Integer settlementDays,    // bond specific
             const Date& issueDate,     // bond specific

             const Calendar& calendar,                // schedule
             const Date& datedDate,                   // schedule
             Frequency couponFrequency,               // schedule
             const Date& maturityDate,                // schedule
             BusinessDayConvention accrualConvention, // schedule

             Real faceAmount, // nominals

             const boost::shared_ptr<IborIndex>& index,
             const DayCounter& paymentDayCounter,

             const Handle<CapletVolatilityStructure>& vol =
                Handle<CapletVolatilityStructure>(),

             Integer fixingDays = Null<Integer>(),
             BusinessDayConvention paymentConvention = Following,

             const std::vector<Real>& gearings = std::vector<Real>(),
             const std::vector<Spread>& spreads = std::vector<Spread>(),

             const std::vector<Rate>& caps = std::vector<Rate>(),
             const std::vector<Rate>& floors = std::vector<Rate>(),

             const Handle<YieldTermStructure>& discountCurve
                                  = Handle<YieldTermStructure>(),

             Real redemption = 100.0,   // bond specific
             const Date& stub = Date(), // schedule
             bool fromEnd = true);      // schedule
    };

}

#endif
