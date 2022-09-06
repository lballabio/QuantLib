/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Chris Kenyon


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


#include <ql/experimental/inflation/cpicapfloorengines.hpp>
#include <ql/experimental/inflation/cpicapfloortermpricesurface.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <utility>


namespace QuantLib {


    InterpolatingCPICapFloorEngine::InterpolatingCPICapFloorEngine(
        Handle<CPICapFloorTermPriceSurface> priceSurf)
    : priceSurf_(std::move(priceSurf)) {
        registerWith(priceSurf_);
    }


    void InterpolatingCPICapFloorEngine::calculate() const
    {
        Real npv = 0.0;

        // what is the difference between the observationLag of the surface
        // and the observationLag of the cap/floor?
        // \TODO next line will fail if units are different
        Period lagDiff = arguments_.observationLag - priceSurf_->observationLag();
        // next line will fail if units are different if Period() is not well written
        QL_REQUIRE(lagDiff >= Period(0, Months), "InterpolatingCPICapFloorEngine: "
                   "lag difference must be non-negative: " << lagDiff);

        // we now need an effective maturity to use in the price surface because this uses
        // maturity of calibration instruments as its time axis, N.B. this must also
        // use the roll because the surface does
        Date effectiveMaturity = arguments_.payDate - lagDiff;


        // what interpolation do we use? Index / flat / linear
        if (arguments_.observationInterpolation == CPI::AsIndex) {
            // same as index means we can just use the price surface
            // since this uses the index
            if (arguments_.type == Option::Call) {
                npv = priceSurf_->capPrice(effectiveMaturity, arguments_.strike);
            } else {
                npv = priceSurf_->floorPrice(effectiveMaturity, arguments_.strike);
            }


        } else {
            std::pair<Date,Date> dd = inflationPeriod(effectiveMaturity, arguments_.infIndex->frequency());
            Real priceStart = 0.0;

            if (arguments_.type == Option::Call) {
                priceStart = priceSurf_->capPrice(dd.first, arguments_.strike);
            } else {
                priceStart = priceSurf_->floorPrice(dd.first, arguments_.strike);
            }

            // if we use a flat index vs the interpolated one ...
            if (arguments_.observationInterpolation == CPI::Flat) {
                // then use the price for the first day in the period because the value cannot change after then
                npv = priceStart;

            } else {
                // linear interpolation will be very close
                Real priceEnd = 0.0;
                if (arguments_.type == Option::Call) {
                    priceEnd = priceSurf_->capPrice((dd.second+Period(1,Days)), arguments_.strike);
                } else {
                    priceEnd = priceSurf_->floorPrice((dd.second+Period(1,Days)), arguments_.strike);
                }

                npv = priceStart + (priceEnd - priceStart) * (effectiveMaturity - dd.first)
                / ( (dd.second+Period(1,Days)) - dd.first); // can't get to next period'
            }

        }
        results_.value = npv;
    }

}
