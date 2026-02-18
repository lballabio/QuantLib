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

    The Kernel template parameter selects the pricing approximation:
    - ZabrShortMaturityLognormal (default): Short maturity lognormal approximation
    - ZabrShortMaturityNormal: Short maturity normal approximation
    - ZabrLocalVolatility: Local volatility approach
    - ZabrFullFd: Full finite difference pricing

    Custom kernels can be used by instantiating
    SwaptionVolCubeZabrModel<MyKernel> and creating a corresponding
    XabrSwaptionVolatilityCube typedef.

    \warning The ZABR model does not support shifted lognormal or normal
             volatility types. The shift and volatilityType parameters from
             the ATM volatility structure are ignored. ZABR always uses its
             native volatility representation based on the selected kernel.

    \section zabr_usage Usage Example

    \code
    // Create a ZABR swaption volatility cube (default kernel)
    std::vector<std::vector<Handle<Quote>>> parametersGuess(n);
    // ... fill with 5 parameters per point: alpha, beta, nu, rho, gamma ...

    std::vector<bool> isParameterFixed = {false, true, false, false, true};

    auto cube = ext::make_shared<ZabrSwaptionVolatilityCube>(
        atmVolStructure, optionTenors, swapTenors, strikeSpreads,
        volSpreads, swapIndexBase, shortSwapIndexBase,
        vegaWeightedSmileFit, parametersGuess, isParameterFixed,
        isAtmCalibrated);

    // Use a different kernel
    typedef XabrSwaptionVolatilityCube<
        SwaptionVolCubeZabrModel<ZabrLocalVolatility>
    > ZabrLocalVolSwaptionVolatilityCube;
    \endcode
*/

#ifndef quantlib_zabr_swaption_volatility_cube_hpp
#define quantlib_zabr_swaption_volatility_cube_hpp

#include <ql/math/comparison.hpp>
#include <ql/termstructures/volatility/swaption/sabrswaptionvolatilitycube.hpp>
#include <ql/math/interpolations/zabrinterpolation.hpp>
#include <ql/termstructures/volatility/zabrsmilesection.hpp>

namespace QuantLib {

    //! ZABR model specification for XabrSwaptionVolatilityCube
    /*! \tparam Kernel The ZABR pricing kernel to use. Available options:
              ZabrShortMaturityLognormal (default), ZabrShortMaturityNormal,
              ZabrLocalVolatility, ZabrFullFd.

        The ZABR model extends SABR with an additional gamma parameter,
        using 5 parameters in total: alpha, beta, nu, rho, gamma.

        \note When gamma = 1.0, the ZABR model closely approximates
              the standard SABR model behavior.
    */
    template <typename Kernel = ZabrShortMaturityLognormal>
    struct SwaptionVolCubeZabrModel {
        typedef ZabrInterpolation<Kernel> Interpolation;
        typedef ZabrSmileSection<Kernel> SmileSection;
    };

    //! Traits specialization for all ZABR model kernels
    /*! Provides 5-parameter support (alpha, beta, nu, rho, gamma)
        and ZABR-specific interpolation/smile section construction.
    */
    template <typename Kernel>
    struct XabrModelTraits<SwaptionVolCubeZabrModel<Kernel>> {
        using Model = SwaptionVolCubeZabrModel<Kernel>;
        static constexpr Size nParams = 5;

        //! Create ZABR interpolation from parameter vectors
        template <class I1, class I2>
        static ext::shared_ptr<typename Model::Interpolation> createInterpolation(
            const I1& xBegin, const I1& xEnd, const I2& yBegin,
            Time t, Real forward,
            const std::vector<Real>& params,
            const std::vector<bool>& paramIsFixed,
            bool vegaWeighted,
            const ext::shared_ptr<EndCriteria>& endCriteria,
            const ext::shared_ptr<OptimizationMethod>& optMethod,
            Real errorAccept, bool useMaxError, Size maxGuesses,
            Real /* shift */, VolatilityType /* volatilityType */) {
            QL_REQUIRE(params.size() >= nParams,
                       "ZABR model requires " << nParams << " parameters, "
                       "but " << params.size() << " provided");
            QL_REQUIRE(paramIsFixed.size() >= nParams,
                       "ZABR model requires " << nParams << " fixed flags, "
                       "but " << paramIsFixed.size() << " provided");
            return ext::make_shared<typename Model::Interpolation>(
                xBegin, xEnd, yBegin, t, forward,
                params[0], params[1], params[2], params[3], params[4],
                paramIsFixed[0], paramIsFixed[1], paramIsFixed[2],
                paramIsFixed[3], paramIsFixed[4],
                vegaWeighted, endCriteria, optMethod,
                errorAccept, useMaxError, maxGuesses);
        }

        //! Extract gamma parameter from calibrated ZABR interpolation
        static Real extractGamma(
            const ext::shared_ptr<typename Model::Interpolation>& interp) {
            return interp->gamma();
        }

        //! Create ZABR smile section from calibrated parameters
        static ext::shared_ptr<typename Model::SmileSection> createSmileSection(
            Time optionTime, Real forward,
            const std::vector<Real>& params,
            Real shift, VolatilityType /* volatilityType */) {
            QL_REQUIRE(close(shift, 0.0),
                       "ZABR model does not support shifted volatilities; "
                       "shift = " << shift << " was provided");
            return ext::make_shared<typename Model::SmileSection>(
                optionTime, forward, params);
        }
    };

    //! ZABR volatility cube for swaptions (default kernel)
    /*! A swaption volatility cube using the ZABR model with 5 parameters
        (alpha, beta, nu, rho, gamma) and ZabrShortMaturityLognormal kernel.

        \see SwaptionVolCubeZabrModel for model specification
        \see XabrModelTraits for customization points
        \see XabrSwaptionVolatilityCube for the generic template
    */
    typedef XabrSwaptionVolatilityCube<SwaptionVolCubeZabrModel<> > ZabrSwaptionVolatilityCube;

}

#endif
