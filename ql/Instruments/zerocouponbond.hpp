/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

/*! \file zerocouponbond.hpp
    \brief zero-coupon bond
*/

#ifndef quantlib_zero_coupon_bond_hpp
#define quantlib_zero_coupon_bond_hpp

#include <ql/Instruments/bond.hpp>

namespace QuantLib {

    //! zero-coupon bond
    /*! \ingroup instruments

        \test calculations are tested by checking results against
              cached values.
    */
    class ZeroCouponBond : public Bond {
      public:
        ZeroCouponBond(const Date& issueDate,
                       const Date& maturityDate,
                       Integer settlementDays,
                       const DayCounter& dayCounter,
                       const Calendar& calendar,
                       BusinessDayConvention convention = Following,
                       Real redemption = 100.0,
                       const Handle<YieldTermStructure>& discountCurve
                                              = Handle<YieldTermStructure>());
    };

}


#endif
