/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Toyin Akin

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

/*! \file bondhelpers.hpp
    \brief bond rate helpers
*/

#ifndef quantlib_bond_helpers_hpp
#define quantlib_bond_helpers_hpp

#include <ql/yieldtermstructures/ratehelper.hpp>
#include <ql/instruments/bonds/fixedratebond.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! fixed-coupon bond helper
    /*! \warning This class assumes that the reference date
                 does not change between calls of setTermStructure().
    */
    class FixedRateBondHelper : public RateHelper {
      public:
        FixedRateBondHelper(const Handle<Quote>& cleanPrice,
                              Natural settlementDays,
                              const Schedule& schedule,
                              const std::vector<Rate>& coupons,
                              const DayCounter& paymentDayCounter,
                              BusinessDayConvention paymentConvention =
                                                                    Following,
                              Real redemption = 100.0,
                              const Date& issueDate = Date());
        Real impliedQuote() const;
        void setTermStructure(YieldTermStructure*);
        boost::shared_ptr<FixedRateBond> bond() const;
        const DayCounter& dayCounter() const;
        Frequency frequency() const;
      protected:
        // needed for bond instantiation
        Natural settlementDays_;
        Schedule schedule_;
        std::vector<Rate> coupons_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentConvention_;
        Real redemption_;
        Date issueDate_;
        // other
        //Date settlement_, latestDate_;
        boost::shared_ptr<FixedRateBond> bond_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };

}


#endif
