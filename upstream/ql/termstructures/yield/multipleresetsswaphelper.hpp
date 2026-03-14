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

/*! \file multipleresetsswaphelper.hpp
    \brief Rate helper based on multiple-resets swap quotes
*/

#ifndef quantlib_multiple_resets_swap_helper_hpp
#define quantlib_multiple_resets_swap_helper_hpp

#include <ql/cashflows/rateaveraging.hpp>
#include <ql/instruments/multipleresetsswap.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>

namespace QuantLib {

    class IborIndex;

    //! Rate helper for bootstrapping from multiple-resets swap quotes
    class MultipleResetsSwapRateHelper : public RelativeDateRateHelper {
      public:
        MultipleResetsSwapRateHelper(
            Natural settlementDays,
            const Period& tenor,
            const std::variant<Rate, Handle<Quote>>& fixedRate,
            const ext::shared_ptr<IborIndex>& iborIndex,
            Size resetsPerCoupon,
            Handle<YieldTermStructure> discountingCurve = {},
            RateAveraging::Type averagingMethod = RateAveraging::Compound,
            Spread spread = 0.0,
            Frequency fixedFrequency = NoFrequency,
            DayCounter fixedDayCount = DayCounter(),
            BusinessDayConvention fixedConvention = ModifiedFollowing);

        Real impliedQuote() const override;
        void setTermStructure(YieldTermStructure*) override;
        void accept(AcyclicVisitor&) override;

      protected:
        void initializeDates() override;

        Natural settlementDays_;
        Period tenor_;
        ext::shared_ptr<IborIndex> iborIndex_;
        Size resetsPerCoupon_;
        RateAveraging::Type averagingMethod_;
        Spread spread_;
        Frequency fixedFrequency_;
        DayCounter fixedDayCount_;
        BusinessDayConvention fixedConvention_;
        Handle<YieldTermStructure> discountHandle_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
        RelinkableHandle<YieldTermStructure> discountRelinkableHandle_;
        ext::shared_ptr<MultipleResetsSwap> swap_;
    };

}

#endif
