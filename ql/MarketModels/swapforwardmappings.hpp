/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file swapforwardmappings.hpp
    \brief Utility functions for mapping between swap rate and forward rate
*/

#ifndef quantlib_swap_forward_mappings_hpp
#define quantlib_swap_forward_mappings_hpp

#include <ql/Math/matrix.hpp>
#include <vector>
#include <ql/MarketModels/curvestate.hpp>

namespace QuantLib {

    class SwapForwardMappings {
      public:

        /*! Returns the dsr[i]/df[j] jacobian between coterminal swap rates
            and forward rates */
        static Disposable<Matrix> coterminalSwapForwardJacobian(const CurveState& cs);

        /*! Returns the Z matrix to switch base from forward to
            coterminal swap rates */
        static Disposable<Matrix> coterminalSwapZedMatrix(const CurveState& cs,
                                                          Spread displacement);

        /*! Returns the dsr[i]/df[j] jacobian between coinitial swap rates
            and forward rates */
        static Disposable<Matrix> coinitialSwapForwardJacobian(const CurveState& cs);

        /*! Returns the Z matrix to switch base from forward to
            coinitial swap rates */
        static Disposable<Matrix> coinitialSwapZedMatrix(const CurveState& cs,
                                                         Spread displacement);
    };

}

#endif
