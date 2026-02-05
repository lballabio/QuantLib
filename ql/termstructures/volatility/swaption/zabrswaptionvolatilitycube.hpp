/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers
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

/*! \file zabrswaptionvolatilitycube.hpp
    \brief Swaption volatility cube using the ZABR model

    The ZABR model is an extension of the SABR model that includes an
    additional gamma parameter for improved smile fitting. This file
    provides the ZabrSwaptionVolatilityCube typedef for building
    swaption volatility cubes using ZABR calibration.

    \section zabr_params ZABR Parameters

    The ZABR model uses 5 parameters:
    - \b alpha (params[0]): Initial volatility level
    - \b beta (params[1]): CEV exponent, typically in [0, 1]
    - \b nu (params[2]): Volatility of volatility
    - \b rho (params[3]): Correlation between forward and volatility
    - \b gamma (params[4]): Additional ZABR parameter (gamma=1 approximates SABR)

    \section zabr_kernels Available ZABR Kernels

    The default implementation uses ZabrShortMaturityLognormal. Alternative
    kernels available for specialized use cases include:
    - ZabrShortMaturityLognormal (default): Short maturity lognormal approximation
    - ZabrShortMaturityNormal: Short maturity normal approximation
    - ZabrLocalVolatility: Local volatility approach
    - ZabrFullFd: Full finite difference pricing

    \warning Unlike SabrSwaptionVolatilityCube, the ZABR model does not support
             shifted lognormal or normal volatility types. The volatilityType
             parameter from the ATM volatility structure is ignored. ZABR always
             uses its native volatility representation based on the selected kernel.

    \section zabr_usage Usage Example

    \code
    // Create a ZABR swaption volatility cube
    std::vector<std::vector<Handle<Quote>>> parametersGuess(n);
    // ... fill with 5 parameters per point: alpha, beta, nu, rho, gamma ...

    std::vector<bool> isParameterFixed = {false, true, false, false, true};

    auto cube = ext::make_shared<ZabrSwaptionVolatilityCube>(
        atmVolStructure, optionTenors, swapTenors, strikeSpreads,
        volSpreads, swapIndexBase, shortSwapIndexBase,
        vegaWeightedSmileFit, parametersGuess, isParameterFixed,
        isAtmCalibrated);
    \endcode
*/

#ifndef quantlib_zabr_swaption_volatility_cube_hpp
#define quantlib_zabr_swaption_volatility_cube_hpp

#include <ql/termstructures/volatility/swaption/sabrswaptionvolatilitycube.hpp>
#include <ql/experimental/volatility/zabrinterpolation.hpp>
#include <ql/experimental/volatility/zabrsmilesection.hpp>

namespace QuantLib {

    //! Swaption Volatility Cube ZABR Model specification
    /*! This struct defines the types used by ZABR Volatility cubes
        for interpolation (ZabrInterpolation) and for modeling the
        smile (ZabrSmileSection).

        The ZABR model extends SABR with an additional gamma parameter,
        using 5 parameters in total: alpha, beta, nu, rho, gamma.
        The default kernel is ZabrShortMaturityLognormal.

        \note When gamma = 1.0, the ZABR model closely approximates
              the standard SABR model behavior.
    */
    struct SwaptionVolCubeZabrModel {
        static const Size nParams = 5;
        typedef ZabrInterpolation<ZabrShortMaturityLognormal> Interpolation;
        typedef ZabrSmileSection<ZabrShortMaturityLognormal> SmileSection;

        //! Create ZABR interpolation from parameter vectors
        /*! \param params Vector of 5 ZABR parameters: alpha, beta, nu, rho, gamma
            \param paramIsFixed Vector of 5 booleans indicating which parameters are fixed
            \note ZABR interpolation does not use shift or volatilityType parameters
        */
        template <class I1, class I2>
        static ext::shared_ptr<Interpolation> createInterpolation(
            const I1& xBegin, const I1& xEnd, const I2& yBegin,
            Time t, Real forward,
            const std::vector<Real>& params,
            const std::vector<bool>& paramIsFixed,
            bool vegaWeighted,
            const ext::shared_ptr<EndCriteria>& endCriteria,
            const ext::shared_ptr<OptimizationMethod>& optMethod,
            Real errorAccept, bool useMaxError, Size maxGuesses,
            Real /* shift */, VolatilityType /* volatilityType */) {
            // Validate parameter vector sizes
            QL_REQUIRE(params.size() >= nParams,
                       "ZABR model requires " << nParams << " parameters, "
                       "but " << params.size() << " provided");
            QL_REQUIRE(paramIsFixed.size() >= nParams,
                       "ZABR model requires " << nParams << " fixed flags, "
                       "but " << paramIsFixed.size() << " provided");
            // ZABR interpolation doesn't use shift or volatilityType
            return ext::make_shared<Interpolation>(
                xBegin, xEnd, yBegin, t, forward,
                params[0], params[1], params[2], params[3], params[4],
                paramIsFixed[0], paramIsFixed[1], paramIsFixed[2],
                paramIsFixed[3], paramIsFixed[4],
                vegaWeighted, endCriteria, optMethod,
                errorAccept, useMaxError, maxGuesses);
        }

        //! Extract gamma parameter from calibrated ZABR interpolation
        static Real extractGamma(const ext::shared_ptr<Interpolation>& interp) {
            return interp->gamma();
        }

        //! Create ZABR smile section from calibrated parameters
        /*! \param optionTime Time to expiry
            \param forward Forward rate
            \param params Vector of 5 ZABR parameters: alpha, beta, nu, rho, gamma
            \param shift Unused for ZABR (included for interface compatibility)
            \param volatilityType Unused for ZABR (included for interface compatibility)
            \note ZABR smile section uses default moneyness grid and fdRefinement
        */
        static ext::shared_ptr<SmileSection> createSmileSection(
            Time optionTime, Real forward,
            const std::vector<Real>& params,
            Real /* shift */, VolatilityType /* volatilityType */) {
            return ext::make_shared<SmileSection>(
                optionTime, forward, params);
        }
    };

    //! ZABR volatility cube for swaptions
    /*! A swaption volatility cube using the ZABR model with 5 parameters
        (alpha, beta, nu, rho, gamma) and ZabrShortMaturityLognormal kernel.

        \see SwaptionVolCubeZabrModel for model specification
        \see XabrSwaptionVolatilityCube for the generic template
    */
    typedef XabrSwaptionVolatilityCube<SwaptionVolCubeZabrModel> ZabrSwaptionVolatilityCube;

}

#endif
