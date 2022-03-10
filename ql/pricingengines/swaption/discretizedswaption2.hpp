/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021, 2022 Ralf Konrad Eckel

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

/*! \file discretizedswaption2.hpp
    \brief Discretized swaption class
*/

#ifndef quantlib_discretized_swaption2_hpp
#define quantlib_discretized_swaption2_hpp

#include <ql/discretizedasset.hpp>
#include <ql/instruments/swaption.hpp>

namespace QuantLib {

    class DiscretizedSwaption2 : public DiscretizedOption {
      public:
        DiscretizedSwaption2(const Swaption::arguments&,
                             const Date& referenceDate,
                             const DayCounter& dayCounter);
        void reset(Size size) override;
        std::vector<Time> mandatoryTimes() const override;

      protected:
        void preAdjustValuesImpl() override;
        void postAdjustValuesImpl() override;

      private:
        enum class CouponAdjustment { pre, post };
        Swaption::arguments arguments_;

        std::vector<Time> fixedResetTimes_;
        std::vector<Time> fixedPayTimes_;
        std::vector<CouponAdjustment> fixedCouponAdjustments_;
        std::vector<Time> floatingResetTimes_;
        std::vector<Time> floatingPayTimes_;
        std::vector<CouponAdjustment> floatCouponAdjustments_;

        std::vector<Real> preCouponAdjustments_;
        std::vector<Real> postCouponAdjustments_;

        bool includeTodaysCashFlows_;
        Time lastPayment_;

        void applyCallability(Size i);
        void addFixedCoupon(Size i);
        void addFloatCoupon(Size i);

        static void
        prepareSwaptionWithSnappedDates(const Swaption::arguments& args,
                                        const Date& referenceDate,
                                        const DayCounter& dayCounter,
                                        PricingEngine::arguments& snappedArgs,
                                        std::vector<Time>& fixedResetTimes,
                                        std::vector<Time>& fixedPayTimes,
                                        std::vector<CouponAdjustment>& fixedCouponAdjustments,
                                        std::vector<Time>& floatingResetTimes,
                                        std::vector<Time>& floatingPayTimes,
                                        std::vector<CouponAdjustment>& floatCouponAdjustments,
                                        std::vector<Real>& preCouponAdjustments,
                                        std::vector<Real>& postCouponAdjustments);
    };

}


#endif
