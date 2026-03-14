/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2007 StatPro Italia srl
 Copyright (C) 2022 Ralf Konrad Eckel

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

/*! \file discretizedswap.hpp
    \brief Discretized swap class
*/

#ifndef quantlib_discretized_swap_hpp
#define quantlib_discretized_swap_hpp

#include <ql/discretizedasset.hpp>
#include <ql/instruments/vanillaswap.hpp>

namespace QuantLib {

    class DiscretizedSwap : public DiscretizedAsset {
      public:
        DiscretizedSwap(const VanillaSwap::arguments& args,
                        const Date& referenceDate,
                        const DayCounter& dayCounter);

        DiscretizedSwap(const VanillaSwap::arguments& args,
                        const Date& referenceDate,
                        const DayCounter& dayCounter,
                        std::vector<CouponAdjustment> fixedCouponAdjustments,
                        std::vector<CouponAdjustment> floatingCouponAdjustments);
        void reset(Size size) override;
        std::vector<Time> mandatoryTimes() const override;

      protected:
        void preAdjustValuesImpl() override;
        void postAdjustValuesImpl() override;

      private:
        VanillaSwap::arguments arguments_;
        std::vector<Time> fixedResetTimes_;
        std::vector<Time> fixedPayTimes_;
        std::vector<CouponAdjustment> fixedCouponAdjustments_;
        std::vector<bool> fixedResetTimeIsInPast_;
        std::vector<Time> floatingResetTimes_;
        std::vector<Time> floatingPayTimes_;
        std::vector<CouponAdjustment> floatingCouponAdjustments_;
        std::vector<bool> floatingResetTimeIsInPast_;

        void addFixedCoupon(Size i);
        void addFloatingCoupon(Size i);
    };

}


#endif
