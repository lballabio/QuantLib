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
}

#endif


#ifndef id_c0b6dbb065e7252b9efa46fb2a3e5bf6
#define id_c0b6dbb065e7252b9efa46fb2a3e5bf6
inline bool test_c0b6dbb065e7252b9efa46fb2a3e5bf6(const int* i) {
    return i != nullptr;
}
#endif
