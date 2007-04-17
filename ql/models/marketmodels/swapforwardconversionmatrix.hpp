/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file swapforwardconversionmatrix.hpp
    \brief Swaption covariance matrix approximation for Market Models
*/

#ifndef quantlib_swap_forward_conversion_matrix_hpp
#define quantlib_swap_forward_conversion_matrix_hpp

#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>
#include <ql/math/matrix.hpp>
#include <vector>

namespace QuantLib {

    //! Swaption covariance matrix approximation for Market Models
    /*! Given the forward covariance matrix, it returns
        an approximation for the corresponding coterminal swap
        covariance matrix. See e.g.:
        -# P. Jackel, R. Rebonato, <i>Linking Caplet and Swaption
           Volatilities in a BGM/J Framework: Approximate Solutions</i>,
           QUARCH preprint, 2000 (http://www.quarchome.org).
        -# R. Rebonato, <i>Modern Pricing of Interest Rate Derivatives</i>,
           Princeton University Press (2002).
        -# M. Joshi, <i>The Concepts and Practice of Mathematical
           Finance</i>, Cambridge University Press (2003).

        \ingroup Market Models
    */
    class SwapCovarianceApproximator {
      public:
        SwapCovarianceApproximator(const CurveState& initialCurveState,
                                   Size expiry,
                                   Size maturity,
                                   Spread displacement,
                                   const Matrix& forwardCovarianceMatrix);

        /*! Given the forward covariance matrix, it returns the approximated
            swap covariance matrix corresponding to the (sub)set of coterminal
            swaps between expiry and maturity.    */
        Disposable<Matrix> swapCovarianceMatrix();

        /*! Returns the subportion of Z matrix corresponding to the segment
            of curve between expiry and maturity.    */
        Disposable<Matrix> zzMatrix();

        /*! Returns the Z matrix to switch base from forward to swap rates   */
        Disposable<Matrix> zMatrix(const CurveState& cs);

        /*! Returns the swap rate / forward rate jacobian dsr[i]/df[j]  */
        Disposable<Matrix> swapForwardJacobian(const CurveState& cs);

        //Real swapVariance(const Matrix& covariance);

      private:
        LMMCurveState initialCurveState_;
        Size expiry_, maturity_;
        Spread displacement_;
        Matrix forwardCovarianceMatrix_, swapCovarianceMatrix_;
    };

}

#endif
