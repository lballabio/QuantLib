
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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

/*! \file inarrearindexedcoupon.hpp
    \brief in arrear indexed coupon

    \fullpath
    ql/CashFlows/%inarrearindexedcoupon.hpp
*/

#ifndef quantlib_in_arrear_indexed_coupon_hpp
#define quantlib_in_arrear_indexed_coupon_hpp

#include <ql/CashFlows/indexedcoupon.hpp>

 
namespace QuantLib {

    namespace CashFlows {

        //! %in arrear indexed coupon class
        /*! \warning This class does not perform any date adjustment,
            i.e., the start and end date passed upon construction
            should be already rolled to a business day.
        */
        class InArrearIndexedCoupon : public IndexedCoupon {
          public:
            InArrearIndexedCoupon(double nominal,
                const Date& paymentDate,
                const Handle<Indexes::Xibor>& index,
                const Date& startDate, const Date& endDate,
                int fixingDays,
                Spread spread = 0.0,
                const Date& refPeriodStart = Date(),
                const Date& refPeriodEnd = Date())
			 : IndexedCoupon(nominal,paymentDate,index,startDate,endDate,fixingDays,spread,refPeriodStart,refPeriodEnd)
			{}
            
			Date fixingDate() const; 
        };


        inline Date InArrearIndexedCoupon::fixingDate() const {
			// fix at the end of period
            return index()->calendar().advance(
                accrualEndDate_, -fixingDays_, Days,
                Preceding);
		}
    }

}


#endif
