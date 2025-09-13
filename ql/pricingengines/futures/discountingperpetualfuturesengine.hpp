/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Hiroto Ogawa

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

/*! \file discountingperpetualfuturesengine.hpp
    \brief discounting perpetual futures engine
*/

#ifndef quantlib_discounting_perpetual_futures_engine_hpp
#define quantlib_discounting_perpetual_futures_engine_hpp

#include <ql/instruments/perpetualfutures.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/math/array.hpp>
#include <ql/handle.hpp>
#include <ql/optional.hpp>
#include <ql/math/interpolation.hpp>

namespace QuantLib {

    //! Discounting engine for swaps
    /*! This engine discounts future swap cashflows to the reference
        date of the discount curve.
    */
    class DiscountingPerpetualFuturesEngine : public PerpetualFutures::engine {
      public:
        enum InterpolationType { PiecewiseConstant, Linear, CubicSpline };
        DiscountingPerpetualFuturesEngine(
            const Handle<YieldTermStructure>& domesticDiscountCurve,
            const Handle<YieldTermStructure>& foreignDiscountCurve,
            const Handle<Quote>& assetSpot,
            const Array fundingTimes,
            const Array fundingRates,
            const Array interestRateDiffs,
            InterpolationType fundingInterpType = PiecewiseConstant);
        void calculate() const override;
        Handle<YieldTermStructure> domsticDiscountCurve() const { return domesticDiscountCurve_; }
        Handle<YieldTermStructure> foreignDiscountCurve() const { return foreignDiscountCurve_; }
        Handle<Quote> assetSpot() const { return assetSpot_; }
        Array fundingTimes() const { return fundingTimes_; }
        Array fundingRates() const { return fundingRates_; }
        Array interestRateDiffs() const { return interestRateDiffs_; }

      private:
        Interpolation selectInterpolation(const Array times, const Array values) const;
        const Handle<YieldTermStructure> domesticDiscountCurve_, foreignDiscountCurve_;
        const Handle<Quote> assetSpot_;
        const Array fundingTimes_, fundingRates_, interestRateDiffs_;
        InterpolationType fundingInterpType_;
        Real maxT_;
    };
}

#endif
