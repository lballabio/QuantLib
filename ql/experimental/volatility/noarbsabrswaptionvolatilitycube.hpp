/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers
 Copyright (C) 2023 Ignacio Anguita
 Copyright (C) 2026 Aaditya Panikath

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

/*! \file noarbsabrswaptionvolatilitycube.hpp
    \brief Swaption volatility cube, fit-early-interpolate-later approach
           using the No Arbitrage Sabr model (Doust)
*/

#ifndef quantlib_noarb_sabr_swaption_volatility_cube_hpp
#define quantlib_noarb_sabr_swaption_volatility_cube_hpp

#include <ql/termstructures/volatility/swaption/sabrswaptionvolatilitycube.hpp>
#include <ql/experimental/volatility/noarbsabrinterpolation.hpp>

namespace QuantLib {

    //! No-Arbitrage SABR model specification for XabrSwaptionVolatilityCube
    struct SwaptionVolCubeNoArbSabrModel {
        typedef NoArbSabrInterpolation Interpolation;
        typedef NoArbSabrSmileSection SmileSection;
    };

    //! Traits specialization for NoArbSabr
    /*! NoArbSabrInterpolation does not accept a volatilityType parameter,
        so we specialize the traits to pass only shift.
    */
    template <>
    struct XabrModelTraits<SwaptionVolCubeNoArbSabrModel> {
        static constexpr Size nParams = 4;

        template <class I1, class I2>
        static ext::shared_ptr<NoArbSabrInterpolation> createInterpolation(
            const I1& xBegin, const I1& xEnd, const I2& yBegin,
            Time t, Real forward,
            const std::vector<Real>& params,
            const std::vector<bool>& paramIsFixed,
            bool vegaWeighted,
            const ext::shared_ptr<EndCriteria>& endCriteria,
            const ext::shared_ptr<OptimizationMethod>& optMethod,
            Real errorAccept, bool useMaxError, Size maxGuesses,
            Real shift, VolatilityType /* volatilityType */) {
            return ext::make_shared<NoArbSabrInterpolation>(
                xBegin, xEnd, yBegin, t, forward,
                params[0], params[1], params[2], params[3],
                paramIsFixed[0], paramIsFixed[1], paramIsFixed[2], paramIsFixed[3],
                vegaWeighted, endCriteria, optMethod,
                errorAccept, useMaxError, maxGuesses, shift);
        }

        static Real extractGamma(
            const ext::shared_ptr<NoArbSabrInterpolation>& /* interp */) {
            return 0.0;
        }

        static ext::shared_ptr<NoArbSabrSmileSection> createSmileSection(
            Time optionTime, Real forward,
            const std::vector<Real>& params,
            Real shift, VolatilityType volatilityType) {
            return ext::make_shared<NoArbSabrSmileSection>(
                optionTime, forward, params, shift, volatilityType);
        }
    };

    //! no-arbitrage SABR volatility cube for swaptions
    typedef XabrSwaptionVolatilityCube<SwaptionVolCubeNoArbSabrModel> NoArbSabrSwaptionVolatilityCube;

}

#endif

