/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo
 Copyright (C) 2021 Ralf Konrad Eckel

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

/*! \file discretizedcallablefixedratebond.hpp
    \brief Discretized callable fixed-rate bond class
*/

#ifndef quantlib_discretized_callable_fixed_rate_bond_hpp
#define quantlib_discretized_callable_fixed_rate_bond_hpp

#include <ql/discretizedasset.hpp>
#include <ql/experimental/callablebonds/callablebond.hpp>

namespace QuantLib {

    class DiscretizedCallableFixedRateBond : public DiscretizedAsset {
      public:
        DiscretizedCallableFixedRateBond(const CallableBond::arguments&,
                                         const Handle<YieldTermStructure>& termStructure);
        void reset(Size size) override;
        std::vector<Time> mandatoryTimes() const override;

      protected:
        void preAdjustValuesImpl() override;
        void postAdjustValuesImpl() override;

      private:
        enum class CouponAdjustment { pre, post };
        CallableBond::arguments arguments_;
        Time redemptionTime_;
        std::vector<Time> couponTimes_;
        std::vector<CouponAdjustment> couponAdjustments_;
        std::vector<Time> callabilityTimes_;
        std::vector<Real> adjustedCallabilityPrices_;
        void applyCallability(Size i);
        void addCoupon(Size i);
    };

}

#endif


#ifndef id_f28a6d55e59dc17b766793a9ef1f3e95
#define id_f28a6d55e59dc17b766793a9ef1f3e95
inline bool test_f28a6d55e59dc17b766793a9ef1f3e95(const int* i) {
    return i != nullptr;
}
#endif
