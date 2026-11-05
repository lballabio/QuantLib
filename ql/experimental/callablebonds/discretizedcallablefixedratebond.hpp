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
 <https://www.quantlib.org/license.shtml>.

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

    //! Discretized callable fixed-rate bond
    /*! The adjustments compare each coupon and callability time with the
        current time of the lattice directly, so the lattice time grid is
        expected to contain every one of those times.  The engine checks
        grids supplied by the caller; grids it builds itself from
        mandatoryTimes() contain them by construction.
    */
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
