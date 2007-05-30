/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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

/*! \file tqreigendecomposition.hpp
    \brief tridiag. QR eigen decomposition with explicite shift aka Wilkinson
*/

#ifndef quantlib_tqr_eigen_decomposition_hpp
#define quantlib_tqr_eigen_decomposition_hpp

#include <ql/math/array.hpp>
#include <ql/math/matrix.hpp>

namespace QuantLib {

    //! tridiag. QR eigen decomposition with explicite shift aka Wilkinson
    /*! References:

        Wilkinson, J.H. and Reinsch, C. 1971, Linear Algebra, vol. II of
        Handbook for Automatic Computation (New York: Springer-Verlag)

        "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery,

        \test the correctness of the result is tested by checking it
              against known good values.
    */
    class TqrEigenDecomposition {
      public:
        enum EigenVectorCalculation { WithEigenVector,
                                      WithoutEigenVector,
                                      OnlyFirstRowEigenVector };

        enum ShiftStrategy { NoShift,
                             Overrelaxation,
                             CloseEigenValue };

        TqrEigenDecomposition(const Array& diag,
                              const Array& sub,
                              EigenVectorCalculation calc = WithEigenVector,
                              ShiftStrategy strategy = CloseEigenValue);

        const Array& eigenvalues()  const { return d_; }
        const Matrix& eigenvectors() const { return ev_; }

        Size iterations() const { return iter_; }

      private:
        bool offDiagIsZero(Size k, Array& e);

        Size iter_;
        Array d_;
        Matrix ev_;
    };

}


#endif
