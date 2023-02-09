/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers
 Copyright (C) 2023 Ignacio Anguita

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

/*! \file noarbsabrswaptionvolatilitycube.hpp
    \brief Swaption volatility cube, fit-early-interpolate-later approach
           using the No Arbitrage Sabr model (Doust)
*/

#ifndef quantlib_noarb_sabr_swaption_volatility_cube_hpp
#define quantlib_noarb_sabr_swaption_volatility_cube_hpp

#include <ql/termstructures/volatility/swaption/sabrswaptionvolatilitycube.hpp>
#include <ql/experimental/volatility/noarbsabrinterpolation.hpp>

namespace QuantLib {

    struct SwaptionVolCubeNoArbSabrModel {
        typedef NoArbSabrInterpolation Interpolation;
        typedef NoArbSabrSmileSection SmileSection;
    };

    //! no-arbitrage SABR volatility cube for swaptions
    typedef XabrSwaptionVolatilityCube<SwaptionVolCubeNoArbSabrModel> NoArbSabrSwaptionVolatilityCube;

    /*! \deprecated Renamed to NoArbSabrSwaptionVolatilityCube.
                    Deprecated in version 1.30. 
    */
    QL_DEPRECATED
    typedef XabrSwaptionVolatilityCube<SwaptionVolCubeNoArbSabrModel> SwaptionVolCube1a;

}

#endif

