/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2015 Peter Caspers
 Copyright (C) 2019 Klaus Spanderen

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

/*! \file sabr.hpp
    \brief SABR functions
*/

#ifndef quantlib_sabr_hpp
#define quantlib_sabr_hpp

#include <ql/types.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>
#include <array>

namespace QuantLib {

    Real unsafeSabrLogNormalVolatility(Rate strike,
                              Rate forward,
                              Time expiryTime,
                              Real alpha,
                              Real beta,
                              Real nu,
                              Real rho);

    Real unsafeShiftedSabrVolatility(Rate strike,
                              Rate forward,
                              Time expiryTime,
                              Real alpha,
                              Real beta,
                              Real nu,
                              Real rho,
                              Real shift,
                              VolatilityType volatilityType = VolatilityType::ShiftedLognormal);

    /* Normal SABR implemented according to
       https://www2.deloitte.com/content/dam/Deloitte/global/Documents/Financial-Services/be-aers-fsi-sabr-sensitivities.pdf
    */
    Real unsafeSabrNormalVolatility(Rate strike,
                                    Rate forward,
                                    Time expiryTime,
                                    Real alpha,
                                    Real beta,
                                    Real nu,
                                    Real rho);

    Real unsafeSabrVolatility(Rate strike,
                              Rate forward,
                              Time expiryTime,
                              Real alpha,
                              Real beta,
                              Real nu,
                              Real rho,
                              VolatilityType volatilityType = VolatilityType::ShiftedLognormal);

    Real sabrVolatility(Rate strike,
                        Rate forward,
                        Time expiryTime,
                        Real alpha,
                        Real beta,
                        Real nu,
                        Real rho,
                        VolatilityType volatilityType = VolatilityType::ShiftedLognormal);

    Real shiftedSabrVolatility(Rate strike,
                                 Rate forward,
                                 Time expiryTime,
                                 Real alpha,
                                 Real beta,
                                 Real nu,
                                 Real rho,
                                 Real shift,
                                 VolatilityType volatilityType = VolatilityType::ShiftedLognormal);

    Real sabrFlochKennedyVolatility(Rate strike,
                                    Rate forward,
                                    Time expiryTime,
                                    Real alpha,
                                    Real beta,
                                    Real nu,
                                    Real rho);

    void validateSabrParameters(Real alpha,
                                Real beta,
                                Real nu,
                                Real rho);

    //! Initial guess for SABR calibration
    /*! See Fabien Le Floc’h and Gary Kennedy, "Explicit SABR Calibration through Simple Expansions",
        available from <https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2467231>.

        The returned array contains the guesses for alpha, beta, nu and rho.  The value for beta
        is the one passed in input.

        The idea is to estimate atm volatility, skew and curvature using the three volatility points
        closest around the forward (k_0 and vol_0 would be the closest strike and its volatility,
        k_m and vol_m the previous point, k_p and vol_p the following one) and solve a system for
        the SABR parameters that match them.
    */
    std::array<Real, 4> sabrGuess(Real k_m, Volatility vol_m,
                                  Real k_0, Volatility vol_0,
                                  Real k_p, Volatility vol_p,
                                  Rate forward,
                                  Time expiryTime,
                                  Real beta,
                                  Real shift,
                                  VolatilityType volatilityType);


}

#endif
