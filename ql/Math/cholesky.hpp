
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file cholesky.hpp
    \brief Cholesky decomposition
*/

#ifndef quantlib_cholesky_h
#define quantlib_cholesky_h

#include <ql/Math/matrix.hpp>

namespace QuantLib {

    //! Cholesky decomposition
    /*! Given a positive-definite symmetric matrix S (positive eigenvalues),
        it calculates the Cholesky decomposition as
        \f[ S = L \cdot L^T \, ,\f]
        where \f$ \cdot \f$ is the standard matrix product
        and  \f$ ^T  \f$ is the transpose operator.

        If invoked with the second parameter equal to true, it can also handle
        positive semi-definite matrices (non-negative eigenvalues).
        DO NOT try it with negative eigenvalue matrices.

        The implementation of the algorithm was inspired by
        Press, Teukolsky, Vetterling, and Flannery,
        "Numerical Recipes in C", 2nd edition, Cambridge
        University Press
    */

    class Cholesky {
      public:
        /* \pre S must be positive (semi)-definite symmetric */
        Cholesky(const Matrix &S,
                 bool flexible = false);
        const Matrix& decomposition() const { return decomposition_; }
      private:
        Matrix decomposition_;
    };

}


#endif

