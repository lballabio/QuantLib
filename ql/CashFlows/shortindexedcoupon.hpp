
/*
 Copyright (C) 2003 Nicolas Di Césaré

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

/*! \file shortindexedcoupon.hpp
    \brief Short (or long) indexed coupon
*/

#ifndef quantlib_short_indexed_coupon_hpp
#define quantlib_short_indexed_coupon_hpp

#include <ql/CashFlows/indexedcoupon.hpp>

namespace QuantLib {

    //! %Short indexed coupon
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */
    template <class IndexedCouponType>
    class Short : public IndexedCouponType {
      public:
        Short(Real nominal,
              const Date& paymentDate,
              const boost::shared_ptr<Xibor>& index,
              const Date& startDate, const Date& endDate,
              Integer fixingDays,
              Spread spread = 0.0,
              const Date& refPeriodStart = Date(),
              const Date& refPeriodEnd = Date(),
              const DayCounter& dayCounter = DayCounter())
        : IndexedCouponType(nominal, paymentDate, index, startDate,
                            endDate, fixingDays, spread,
                            refPeriodStart, refPeriodEnd, dayCounter) {}
        //! inhibit calculation
        /*! Unlike ParCoupon, this coupon can't calculate
            its fixing for future dates, either.
        */
        Real amount() const {
            QL_FAIL("short/long indexed coupons not supported yet"
                    " (start = " +
                    DateFormatter::toString(accrualStartDate_) +
                    ", end = " +
                    DateFormatter::toString(accrualEndDate_) + ")");
        }
    };

}


#endif
