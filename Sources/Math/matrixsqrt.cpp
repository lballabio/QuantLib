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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/
/*! \file matrixsqrt.cpp
    \brief Returns the square root of a real symmetric matrix

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/01/24 13:16:35  marmar
    sqrt for Matrix

*/

#include "matrixsqrt.h"
#include "jacobidecomposition.h"
#include "dataformatters.h"

namespace QuantLib {
  
  namespace Math {
    
      Matrix sqrt(const Matrix &realSymmMatrix){
        
          //! eigenvalues smaller than tollerance are considered zero
          const double tollerance = 1e-15;
          
          Matrix inputMatrix(realSymmMatrix);
          int size = inputMatrix.rows();
    
          QL_REQUIRE(size == inputMatrix.columns(),
              "sqrt input matrix must be square");

          JacobiDecomposition jd(inputMatrix);
          Matrix evectors(jd.eigenvectors());
          Array evalues(jd.eigenvalues());
          
          double maxEv=0;
          for(int i = 0; i < size;i++)
              if(evalues[i] >= maxEv) 
                  maxEv = evalues[i];                
          
          Matrix diagonal(size,size,0);
          for(int i = 0; i < size;i++){
              if(QL_FABS(evalues[i]) <= tollerance*maxEv) 
                  evalues[i] = 0;
              QL_REQUIRE(evalues[i] >=0, 
                  "sqrt: some eigenvalues are negative: ("+
                  DoubleFormatter::toString(i)+") = "
                  + DoubleFormatter::toString(evalues[i]));
              diagonal[i][i] = QL_SQRT(evalues[i]);
          }
          
          Matrix result(evectors * diagonal * transpose(evectors));
          return result;
      }
  
  }

}
