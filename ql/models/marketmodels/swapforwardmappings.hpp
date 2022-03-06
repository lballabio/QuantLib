/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006, 2008 Mark Joshi

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

/*! \file swapforwardmappings.hpp
    \brief Utility functions for mapping between swap rate and forward rate
*/

#ifndef quantlib_swap_forward_mappings_hpp
#define quantlib_swap_forward_mappings_hpp

#include <ql/math/matrix.hpp>

namespace QuantLib {

    class CurveState;
    class MarketModel;

    class SwapForwardMappings {
      public:

        //! compute annuity of arbitrary swap-rate
        static Real annuity(const CurveState& cs,
                            Size startIndex,
                            Size endIndex,
                            Size numeraireIndex);

        //! compute derivative of swap-rate to underlying forward rate
        static Real swapDerivative(const CurveState& cs,
                                   Size startIndex,
                                   Size endIndex,
                                   Size forwardIndex);

        /*! Returns the dsr[i]/df[j] jacobian between
            coterminal swap rates and forward rates */
        static Disposable<Matrix>
        coterminalSwapForwardJacobian(const CurveState& cs);

        /*! Returns the Z matrix to switch base from forward to
            coterminal swap rates */
        static Disposable<Matrix> coterminalSwapZedMatrix(const CurveState& cs,
                                                          Spread displacement);

        /*! Returns the dsr[i]/df[j] jacobian between
            coinitial swap rates and forward rates */
        static Disposable<Matrix>
        coinitialSwapForwardJacobian(const CurveState& cs);

        /*! Returns the Z matrix to switch base from forward to
            coinitial swap rates */
        static Disposable<Matrix> coinitialSwapZedMatrix(const CurveState& cs, Spread displacement);


        /*! Returns the dsr[i]/df[j] jacobian between
            constant maturity swap rates and forward rates */
        static Disposable<Matrix> cmSwapForwardJacobian(const CurveState& cs,
                                                        Size spanningForwards);

        /*! Returns the Z matrix to switch base from forward to
            constant maturity swap rates */
        static Disposable<Matrix>
        cmSwapZedMatrix(const CurveState& cs, Size spanningForwards, Spread displacement);

        /*! computes the implied vol of a swaption specified by two indices
        using the freezing coefficients methdodology. This routine is easy to use but not very efficient
        and if you want to do a lot of cases, then a different approach should be used. 

        Tested in SwapForwardMappingsTest::testSwaptionImpliedVolatility() in swapforwardmappings.cpp
        */
        static Real 
        swaptionImpliedVolatility(const MarketModel& volStructure,
                                  Size startIndex,
                                  Size endIndex);

    };

}

#endif


#ifndef id_a77259bd226db6fe14d4306a1ed506ed
#define id_a77259bd226db6fe14d4306a1ed506ed
inline bool test_a77259bd226db6fe14d4306a1ed506ed(const int* i) {
    return i != nullptr;
}
#endif
