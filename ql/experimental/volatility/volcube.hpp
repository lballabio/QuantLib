/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

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

/*! \file volcube.hpp
    \brief Interest rate (optionlet/swaption) volatility cube
*/

#ifndef quantlib_volatility_cube_h
#define quantlib_volatility_cube_h

#include <ql/handle.hpp>
#include <ql/shared_ptr.hpp>
#include <vector>

namespace QuantLib {

    class Period;
    class AbcdAtmVolCurve;
    class InterestRateVolSurface;
    class InterestRateIndex;

    class VolatilityCube {
      public:
        VolatilityCube(std::vector<Handle<InterestRateVolSurface> >,
                       std::vector<Handle<AbcdAtmVolCurve> >);
        const Period& minIndexTenor() const;
        const Period& maxIndexTenor() const;
        const std::vector<Handle<InterestRateVolSurface> >& surfaces() const;
        const std::vector<Handle<AbcdAtmVolCurve> >& curves() const;
      protected:
        std::vector<Handle<InterestRateVolSurface> > surfaces_;
        std::vector<Handle<AbcdAtmVolCurve> > curves_;
    };

    // inline

    inline const std::vector<Handle<InterestRateVolSurface> >&
    VolatilityCube::surfaces() const {
        return surfaces_;
    }

    inline const std::vector<Handle<AbcdAtmVolCurve> >&
    VolatilityCube::curves() const {
        return curves_;
    }

}

#endif


#ifndef id_dff096670f30153b30bb1e8ea8fceaf7
#define id_dff096670f30153b30bb1e8ea8fceaf7
inline bool test_dff096670f30153b30bb1e8ea8fceaf7(int* i) { return i != 0; }
#endif
