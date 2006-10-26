/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) Mark Joshi 2006

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


#ifndef quantlib_swap_forward_conversion_matrix_hpp
#define quantlib_swap_forward_conversion_matrix_hpp

#include <ql/Math/matrix.hpp>
#include <ql/MarketModels/curvestate.hpp>

namespace QuantLib {

    //! Swaption covariance matrix approximation for Market Models
    /*! \ingroup Market Models */

    Disposable<Matrix> swaptionsCovarianceMatrix(
                                    const Matrix& zMatrix, 
                                    const Matrix& forwardCovarianceMatrix);
    Disposable<Matrix> coefficientsCsi(const CurveState& cs, Size N, Size M);
    Disposable<Matrix> zMatrix(const CurveState&, Rate displacement);
    Disposable<Matrix> swapForwardJacobian(const CurveState& cs);

    class SwaptionVarianceApproximator {
      public:
        /*! Given the covariance matrix of caplets, 
            it returns an approximation for the corresponding
            covariance matrix for coterminal swaptions.
            See e.g.:
            [1] P. Jackel, R. Rebonato, "\it {Linking Caplet and Swaption
                Volatilities in a BGM/J Framework: Approximate Solutions}", 
                QUARCH preprint, 2000 (http://www.quarchome.org).
            [2] R. Rebonato, "\it {Modern Pricing of Interest Rate Derivatives}",
                Princeton University Press (2002).
            [3] M. Joshi, "\it {The Concepts and Practice of Mathematical 
                Finance}", Cambridge University Press (2003).
        */
          SwaptionVarianceApproximator(const CurveState& initialState,
                                     Real displacement,
                                     Size optionExpiry,
                                     Size swapTenor);
          Real swaptionVariance(const Matrix& covariance);
    };

}

#endif
