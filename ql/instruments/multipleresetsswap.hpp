/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Zain Mughal

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file multipleresetsswap.hpp
    \brief Swap with a multiple-resets floating leg
*/

#ifndef quantlib_multiple_resets_swap_hpp
#define quantlib_multiple_resets_swap_hpp

#include <ql/cashflows/rateaveraging.hpp>
#include <ql/instruments/fixedvsfloatingswap.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    class IborIndex;

    //! Swap with a fixed leg and a multiple-resets floating leg
    /*! The floating leg contains coupons whose rate is determined by
        compounding or averaging \c resetsPerCoupon consecutive Ibor
        fixings during each accrual period.
    */
    class MultipleResetsSwap : public FixedVsFloatingSwap {
      public:
        MultipleResetsSwap(Type type,
                           Real nominal,
                           const Schedule& fixedSchedule,
                           Rate fixedRate,
                           DayCounter fixedDayCount,
                           Schedule fullResetSchedule,
                           const ext::shared_ptr<IborIndex>& iborIndex,
                           Size resetsPerCoupon,
                           Spread spread = 0.0,
                           RateAveraging::Type averagingMethod = RateAveraging::Compound,
                           ext::optional<BusinessDayConvention> paymentConvention = ext::nullopt,
                           Integer paymentLag = 0,
                           const Calendar& paymentCalendar = Calendar());

        const Schedule& fullResetSchedule() const { return fullResetSchedule_; }
        Size resetsPerCoupon() const { return resetsPerCoupon_; }
        RateAveraging::Type averagingMethod() const { return averagingMethod_; }

      private:
        void setupFloatingArguments(arguments* args) const override;

        Schedule fullResetSchedule_;
        Size resetsPerCoupon_;
        RateAveraging::Type averagingMethod_;
    };

}

#endif
