
/*
 Copyright (C) 2003 Nicolas Di Césaré

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file upfrontindexedcoupon.hpp
    \brief Up front indexed coupon

    \fullpath
    ql/CashFlows/%upfrontindexedcoupon.hpp
*/

#ifndef quantlib_up_front_indexed_coupon_hpp
#define quantlib_up_front_indexed_coupon_hpp

#include <ql/CashFlows/indexedcoupon.hpp>

namespace QuantLib {

    namespace CashFlows {

        //! %up front indexed coupon class
        /*! \warning This class does not perform any date adjustment,
            i.e., the start and end date passed upon construction
            should be already rolled to a business day.
        */
        class UpFrontIndexedCoupon : public IndexedCoupon {
          public:
            UpFrontIndexedCoupon(double nominal,
                const Date& paymentDate,
                const Handle<Indexes::Xibor>& index,
                const Date& startDate, const Date& endDate,
                int fixingDays,
                Spread spread = 0.0,
                const Date& refPeriodStart = Date(),
                const Date& refPeriodEnd = Date())
			 : IndexedCoupon(nominal, paymentDate, index, startDate, endDate,
			   fixingDays, spread, refPeriodStart, refPeriodEnd) {}
			Date fixingDate() const;
        };

		inline Date UpFrontIndexedCoupon::fixingDate() const {
			// fix at the beginning of period
            return index()->calendar().advance(
                accrualStartDate_, -fixingDays_, Days,
                Preceding);
        }
    }

}


#endif
