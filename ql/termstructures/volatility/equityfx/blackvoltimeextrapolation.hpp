/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 AcadiaSoft Inc.
 Copyright (C) 2026 Paolo D'Elia

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

/*! \file blackvoltimeextrapolation.hpp
    \brief Time extrapolation of Black volatility term structures
*/

#ifndef quantlib_black_vol_time_extrapolation_hpp
#define quantlib_black_vol_time_extrapolation_hpp

#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    //! Time-extrapolation strategies for Black volatility term structures.
    /*! This class provides static methods to extrapolate variance
        beyond the maximum time of a BlackVolTermStructure, supporting both
        surface (strike-dependent) and curve (ATM) cases.

        Different extrapolation strategies are available via the Type enum.
    */
    class BlackVolTimeExtrapolation {
      public:
        //! Time-extrapolation strategy for Black volatility
        enum Type {
            FlatVolatility,    /*!< Flat extrapolation of the latest available volatility. */
            UseInterpolator,   /*!< Delegate extrapolation to the underlying curve or surface,
                                    whatever the method it uses. */
            LinearVariance     /*!< Linear extrapolation of variance from the last two
                                    available nodes. */
        };

        //! extrapolate a surface to a given time and strike
        static Real extrapolatedVariance(Type type, Time t, Real strike, const std::vector<Time>& times,
                                         const std::function<Real(Time t, Real k)>& varianceSurface);
        //! extrapolate an ATM curve to a given time
        static Real extrapolatedVariance(Type type, Time t, const std::vector<Time>& times,
                                         const std::function<Real(Time t)>& varianceCurve);
    };

}


#endif
