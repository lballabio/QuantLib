/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file matrix.cpp
    \brief matrix used in linear algebra.

    $Source$
    $Name$
    $Log$
    Revision 1.5  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.4  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.3  2001/04/04 06:21:35  marmar
    Error messages changed

    Revision 1.2  2001/02/16 15:14:07  lballabio
    renamed sqrt to matrixSqrt

    Revision 1.1  2001/01/25 11:55:12  lballabio
    *** empty log message ***

*/

#include "Math/matrix.hpp"
#include "Math/symmetricschurdecomposition.hpp"
#include "dataformatters.hpp"

namespace QuantLib {
  
  namespace Math {
    
      Matrix matrixSqrt(const Matrix &realSymmMatrix) {
        
          //! eigenvalues smaller than tolerance are considered zero
          const double tolerance = 1e-15;
          
          Matrix inputMatrix(realSymmMatrix);
          int size = inputMatrix.rows();
    
          QL_REQUIRE(size == inputMatrix.columns(),
              "sqrt input matrix must be square");

          SymmetricSchurDecomposition jd(inputMatrix);
          Matrix evectors(jd.eigenvectors());
          Array evalues(jd.eigenvalues());
          
          double maxEv=0;
		  int i;
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
