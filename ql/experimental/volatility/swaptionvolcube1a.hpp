/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

/*! \file swaptionvolcube1a.hpp
    \brief Swaption volatility cube, fit-early-interpolate-later approach
           using the No Arbitrage Sabr model (Doust)
*/

#ifndef quantlib_swaption_volcube_1a_hpp
#define quantlib_swaption_volcube_1a_hpp

#include <ql/termstructures/volatility/swaption/swaptionvolcube1.hpp>
#include <ql/experimental/volatility/noarbsabrinterpolation.hpp>

namespace QuantLib {

    struct SwaptionVolCubeNoArbSabrModel {
        typedef NoArbSabrInterpolation Interpolation;
        typedef NoArbSabrSmileSection SmileSection;
    };

    typedef SwaptionVolCube1x<SwaptionVolCubeNoArbSabrModel> SwaptionVolCube1a;

}

#endif



#ifndef id_ad49a4c34f680803f85720e7ec45afde
#define id_ad49a4c34f680803f85720e7ec45afde
inline bool test_ad49a4c34f680803f85720e7ec45afde(int* i) { return i != 0; }
#endif
