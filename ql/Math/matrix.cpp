
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file matrix.cpp
    \brief matrix used in linear algebra.

    \fullpath
    ql/Math/%matrix.cpp
*/

// $Id$

#include <ql/Math/symmetricschurdecomposition.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

  namespace Math {

      Matrix matrixSqrt(const Matrix &realSymmMatrix) {

          //! eigenvalues smaller than tolerance are considered zero
          const double tolerance = 1e-15;

          Matrix inputMatrix(realSymmMatrix);
          Size size = inputMatrix.rows();

          QL_REQUIRE(size == inputMatrix.columns(),
              "sqrt input matrix must be square");

          SymmetricSchurDecomposition jd(inputMatrix);
          Matrix evectors(jd.eigenvectors());
          Array evalues(jd.eigenvalues());

          double maxEv=0;
          Size i;
          for(i = 0; i < size;i++)
              if(evalues[i] >= maxEv)
                  maxEv = evalues[i];

          Matrix diagonal(size,size,0);
          for(i = 0; i < size;i++){
              if(QL_FABS(evalues[i]) <= tolerance*maxEv)
                  evalues[i] = 0;
              QL_ENSURE(evalues[i] >=0,
                  "sqrt: some eigenvalues("+
                  IntegerFormatter::toString(i) + ") = " +
                  DoubleFormatter::toString(evalues[i]) +
                  " are negative!");
              diagonal[i][i] = QL_SQRT(evalues[i]);
          }

          Matrix result(evectors * diagonal * transpose(evectors));
          return result;
      }

  }

}
