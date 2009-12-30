/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud

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


#ifndef quantlib_tap_correlations_hpp
#define quantlib_tap_correlations_hpp

#include <ql/types.hpp>
#include <ql/utilities/disposable.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <boost/function.hpp>
#include <vector>

namespace QuantLib {

    //! Returns the Triangular Angles Parametrized correlation matrix
    /*! The matrix \f$ m \f$ is filled with values corresponding to angles
        given in the \f$ angles \f$ vector. See equation (24) in
        "Parameterizing correlations: a geometric interpretation"
        by Francesco Rapisarda, Damiano Brigo, Fabio Mercurio

        \test
        - the correctness of the results is tested by reproducing
          known good data.
        - the correctness of the results is tested by checking
          returned values against numerical calculations.
    */
    Disposable<Matrix>
    triangularAnglesParametrization(const Array& angles,
                                    Size matrixSize,
                                    Size rank);

    Disposable<Matrix>
    lmmTriangularAnglesParametrization(const Array& angles,
                                       Size matrixSize,
                                       Size rank);

    // the same function using the angles parameterized by the following
    // transformation \f[ \teta_i = \frac{\Pi}{2} - arctan(x_i)\f]
    Disposable<Matrix>
    triangularAnglesParametrizationUnconstrained(const Array& x,
                                                 Size matrixSize,
                                                 Size rank);

    Disposable<Matrix>
    lmmTriangularAnglesParametrizationUnconstrained(const Array& x,
                                                    Size matrixSize,
                                                    Size rank);


    //! Returns the rank reduced Triangular Angles Parametrized correlation matrix
    /*! The matrix \f$ m \f$ is filled with values corresponding to angles
        corresponding  to the 3D spherical spiral paramterized by
        \f$ alpha \f$, \f$ t0 \f$, \f$ epsilon \f$ values. See equation (32) in
        "Parameterizing correlations: a geometric interpretation"
        by Francesco Rapisarda, Damiano Brigo, Fabio Mercurio

        \test
        - the correctness of the results is tested by reproducing
          known good data.
        - the correctness of the results is tested by checking
          returned values against numerical calculations.
    */
    Disposable<Matrix>
    triangularAnglesParametrizationRankThree(Real alpha,
                                             Real t0,
                                             Real epsilon,
                                             Size nbRows);

    // the same function with parameters packed in an Array
    Disposable<Matrix>
    triangularAnglesParametrizationRankThreeVectorial(const Array& paramters,
                                                      Size nbRows);

    // Cost function associated with Frobenius norm.
    // <http://en.wikipedia.org/wiki/Matrix_norm>
    class FrobeniusCostFunction : public CostFunction{
      public:
        FrobeniusCostFunction(
            const Matrix& target,
            const boost::function<Disposable<Matrix>(const Array&,
                                                     Size,
                                                     Size)>& f,
                                                     Size matrixSize,
                                                     Size rank)
        : target_(target), f_(f), matrixSize_(matrixSize), rank_(rank) {}
        Real value (const Array &x) const;
        Disposable<Array> values (const Array &x) const;
      private:
        Matrix target_;
        boost::function<Disposable<Matrix>(const Array&, Size, Size)> f_;
        Size matrixSize_;
        Size rank_;
    };
}

#endif
