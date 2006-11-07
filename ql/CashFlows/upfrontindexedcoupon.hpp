/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Nicolas Di Césaré

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

/*! \file upfrontindexedcoupon.hpp
    \brief Up front indexed coupon
*/

#ifndef quantlib_up_front_indexed_coupon_hpp
#define quantlib_up_front_indexed_coupon_hpp

#include <ql/CashFlows/floatingratecoupon.hpp>

namespace QuantLib {

    //! %up front indexed coupon class
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */
    class UpFrontIndexedCoupon : public FloatingRateCoupon {
      public:
        UpFrontIndexedCoupon(const Date& paymentDate,
                             const Real nominal,
                             const Date& startDate,
                             const Date& endDate,
                             const Integer fixingDays,
                             const boost::shared_ptr<InterestRateIndex>& index,
                             const Real gearing = 1.0,
                             const Spread spread = 0.0,
                             const Date& refPeriodStart = Date(),
                             const Date& refPeriodEnd = Date(),
                             const DayCounter& dayCounter = DayCounter())
        : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                             fixingDays, index, gearing, spread,
                             refPeriodStart, refPeriodEnd, dayCounter) {}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
    };


    // inline definitions

    inline void UpFrontIndexedCoupon::accept(AcyclicVisitor& v) {
        Visitor<UpFrontIndexedCoupon>* v1 =
            dynamic_cast<Visitor<UpFrontIndexedCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

}

#endif
