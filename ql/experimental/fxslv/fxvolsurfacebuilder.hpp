/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

/*! \file fxvolsurfacebuilder.hpp
    \brief FX vol surfaces and smile sections from quotes, with the model chosen at run time
*/

#ifndef quantlib_fx_vol_surface_builder_hpp
#define quantlib_fx_vol_surface_builder_hpp

#include <ql/experimental/fxslv/conventions/fxpairconventions.hpp>
#include <ql/experimental/fxslv/fxeventvariance.hpp>
#include <ql/experimental/fxslv/smilesections/fxsmilesection.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <vector>

namespace QuantLib {

    //! The smile model of the sections a surface is built from
    enum class FxSmileModel { Sabr, Svi, Polynomial, Quadratic };

    //! How a surface interpolates between its pillars
    enum class FxVolInterpolation {
        Ncp,   //!< linear in normalised call price (fxVarianceSurfaceNCP)
        Clark  //!< linear in variance (fxVarianceSurfaceClark)
    };

    //! How a vol surface is built from its quotes: modelling choices, not conventions
    struct FxVolSurfaceConfig {
        explicit FxVolSurfaceConfig(FxSmileModel model = FxSmileModel::Sabr,
                                    FxVolInterpolation interpolation = FxVolInterpolation::Ncp,
                                    Real holidayWeight = 0.2)
        : model(model), interpolation(interpolation), holidayWeight(holidayWeight) {
            QL_REQUIRE(holidayWeight > 0.0 && holidayWeight <= 1.0,
                       "holiday weight " << holidayWeight << " not in (0, 1]");
        }
        FxSmileModel model;
        FxVolInterpolation interpolation;
        //! Trading-time weight of non-business days, in (0, 1].
        Real holidayWeight;
    };

    //! The vol quotes of a pair, by tenor
    /*! In the order of the conventions' vol tenors, and the risk reversals
        and butterflies by delta in the order of the conventions' deltas;
        the butterflies are of the conventions' fly type.
    */
    struct FxVolQuotes {
        FxVolQuotes() = default;
        FxVolQuotes(std::vector<Volatility> atms,
                    std::vector<std::vector<Volatility>> riskReversals,
                    std::vector<std::vector<Volatility>> butterflies)
        : atms(std::move(atms)), riskReversals(std::move(riskReversals)),
          butterflies(std::move(butterflies)) {}
        std::vector<Volatility> atms;
        std::vector<std::vector<Volatility>> riskReversals, butterflies;
    };

    //! The variance surface of a pair from its quotes
    /*! The pillars are the expiries of the conventions' vol tenors on
        the as-of date, on the trading-time clock of the convention's
        calendar.  With pending events, the clock releases their variance
        on the surface's ATM pillars, i.e. the fitted smiles' ATMs, which
        need not reprice the quotes exactly.

        The surface returned implements FxVolSurface.  Its reference date
        is the as-of date, fixed: it does not move with the evaluation date.
    */
    ext::shared_ptr<BlackVolTermStructure>
    makeFxVolSurface(const Date& asOf,
                     const FxPairConventions& conventions,
                     const Handle<Quote>& spot,
                     const FxVolQuotes& quotes,
                     const Handle<YieldTermStructure>& foreignCurve,
                     const Handle<YieldTermStructure>& domesticCurve,
                     const std::vector<FxVolEvent>& events,
                     const FxVolSurfaceConfig& config);

    //! A smile section of the given model at an expiry, from its quotes
    /*! The butterflies are of the conventions' fly type.  The section's
        reference date is the as-of date.
    */
    ext::shared_ptr<FxSmileSection>
    makeFxSmileSection(const Date& asOf,
                       const FxPairConventions& conventions,
                       const Date& expiry,
                       const Handle<Quote>& spot,
                       Volatility atm,
                       const std::vector<Volatility>& riskReversals,
                       const std::vector<Volatility>& butterflies,
                       const Handle<YieldTermStructure>& foreignCurve,
                       const Handle<YieldTermStructure>& domesticCurve,
                       FxSmileModel model);

}

#endif
