/*
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2004 M-Dimension Consulting Inc.

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fixedcouponbond.hpp
    \brief fixed-coupon bond
*/

#ifndef quantlib_fixed_coupon_bond_hpp
#define quantlib_fixed_coupon_bond_hpp

#include <ql/Instruments/bond.hpp>

namespace QuantLib {

    //! fixed-coupon bond
    /*! \ingroup instruments */
    class FixedCouponBond : public Bond {
      public:
        FixedCouponBond(const Date& issueDate,
                        const Date& datedDate,
                        const Date& maturityDate,
                        Integer settlementDays,
                        Rate coupon,
                        Frequency couponFrequency,
                        const DayCounter& dayCounter,
                        const Calendar& calendar,
                        BusinessDayConvention convention = Following,
                        Real redemption = 100.0);
    };

}


#endif
