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

/*! \file makemultipleresetsswap.hpp
    \brief Helper class to instantiate multiple-resets swaps
*/

#ifndef quantlib_make_multiple_resets_swap_hpp
#define quantlib_make_multiple_resets_swap_hpp

#include <ql/cashflows/rateaveraging.hpp>
#include <ql/instruments/multipleresetsswap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/dategenerationrule.hpp>

namespace QuantLib {

    //! helper class
    /*! Provides a more comfortable way to instantiate multiple-resets swaps.
        By default the fixed leg frequency matches the coupon period implied
        by the index tenor and \c resetsPerCoupon.
    */
    class MakeMultipleResetsSwap {
      public:
        MakeMultipleResetsSwap(const Period& tenor,
                               const ext::shared_ptr<IborIndex>& iborIndex,
                               Size resetsPerCoupon,
                               Rate fixedRate = Null<Rate>(),
                               const Period& fwdStart = 0 * Days);

        operator MultipleResetsSwap() const;
        operator ext::shared_ptr<MultipleResetsSwap>() const;

        MakeMultipleResetsSwap& receiveFixed(bool flag = true);
        MakeMultipleResetsSwap& withType(Swap::Type type);
        MakeMultipleResetsSwap& withNominal(Real n);
        MakeMultipleResetsSwap& withSettlementDays(Natural settlementDays);
        MakeMultipleResetsSwap& withEffectiveDate(const Date&);
        MakeMultipleResetsSwap& withTerminationDate(const Date&);
        MakeMultipleResetsSwap& withFixedLegFrequency(Frequency f);
        MakeMultipleResetsSwap& withFixedLegDayCount(const DayCounter& dc);
        MakeMultipleResetsSwap& withFixedLegConvention(BusinessDayConvention bdc);
        MakeMultipleResetsSwap& withFloatingLegSpread(Spread sp);
        MakeMultipleResetsSwap& withAveragingMethod(RateAveraging::Type m);
        MakeMultipleResetsSwap& withDiscountingTermStructure(
                               const Handle<YieldTermStructure>&);
        MakeMultipleResetsSwap& withPricingEngine(
                               const ext::shared_ptr<PricingEngine>&);

      private:
        Period tenor_;
        ext::shared_ptr<IborIndex> iborIndex_;
        Size resetsPerCoupon_;
        Rate fixedRate_;
        Period forwardStart_;

        Natural settlementDays_ = Null<Natural>();
        Date effectiveDate_, terminationDate_;
        Swap::Type type_ = Swap::Payer;
        Real nominal_ = 1.0;
        Frequency fixedFrequency_ = NoFrequency;
        DayCounter fixedDayCount_;
        BusinessDayConvention fixedConvention_ = ModifiedFollowing;
        Spread spread_ = 0.0;
        RateAveraging::Type averagingMethod_ = RateAveraging::Compound;
        ext::shared_ptr<PricingEngine> engine_;
    };

}

#endif
