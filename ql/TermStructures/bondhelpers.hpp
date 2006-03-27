/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Toyin Akin

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

/*! \file bondhelpers.hpp
    \brief bond rate helpers
*/

#ifndef quantlib_bond_helpers_hpp
#define quantlib_bond_helpers_hpp

#include <ql/Instruments/fixedcouponbond.hpp>
#include <ql/TermStructures/ratehelpers.hpp>

namespace QuantLib {

    //! fixed-coupon bond helper
    /*! \warning This class assumes that the reference date
                 does not change between calls of setTermStructure().
    */
    class FixedCouponBondHelper : public RateHelper {
      public:
        FixedCouponBondHelper(const Handle<Quote>& cleanPrice,
                              const Date& issueDate,
                              const Date& datedDate,
                              const Date& maturityDate,
                              Integer settlementDays,
                              const std::vector<Rate>& coupons,
                              Frequency frequency,
                              const Calendar& calendar,
                              const DayCounter& dayCounter,
                              BusinessDayConvention accrualConvention =
                                                                    Following,
                              BusinessDayConvention paymentConvention =
                                                                    Following,
                              Real redemption = 100.0,
                              const Date& stub = Date(),
                              bool fromEnd = true);
        Real impliedQuote() const;
        Date latestDate() const;
        void setTermStructure(YieldTermStructure*);
      protected:
        Date issueDate_, datedDate_, maturityDate_;
        Integer settlementDays_;
        std::vector<Rate> coupons_;
        Frequency frequency_;
        DayCounter dayCounter_;
        Calendar calendar_;
        BusinessDayConvention accrualConvention_, paymentConvention_;
        Real redemption_;
        Date stub_;
        bool fromEnd_;
        Date settlement_, latestDate_;
        boost::shared_ptr<FixedCouponBond> bond_;
        Handle<YieldTermStructure> termStructureHandle_;
    };

}


#endif
