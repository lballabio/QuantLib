
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file matrix.cpp

    \fullpath
    Sources/Math/%matrix.cpp
    \brief matrix used in linear algebra.

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 13:57:12  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.16  2001/08/31 15:23:47  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.15  2001/08/28 12:30:51  nando
// unsigned int instead of int
//
// Revision 1.14  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.13  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.12  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.11  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.10  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.9  2001/05/24 15:40:09  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Math/symmetricschurdecomposition.hpp"
#include "ql/dataformatters.hpp"

namespace QuantLib {

  namespace Math {

      Matrix matrixSqrt(const Matrix &realSymmMatrix) {

          //! eigenvalues smaller than tolerance are considered zero
          const double tolerance = 1e-15;

          Matrix inputMatrix(realSymmMatrix);
          unsigned int size = inputMatrix.rows();

          QL_REQUIRE(size == inputMatrix.columns(),
              "sqrt input matrix must be square");

          SymmetricSchurDecomposition jd(inputMatrix);
          Matrix evectors(jd.eigenvectors());
          Array evalues(jd.eigenvalues());

          double maxEv=0;
		  unsigned int i;
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
