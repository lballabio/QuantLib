
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

/*! \file outerproduct.h
    \brief Returns a Matrix that is the outer product of two vectors
    
    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/01/24 09:22:46  marmar
    Returns a Matrix that is the outer product of two vectors

        
*/

#ifndef quantlib_math_outer_product_h
#define quantlib_math_outer_product_h

#include "qldefines.h"
#include "array.h"
#include "matrix.h"

namespace QuantLib {

    namespace Math {
      
    //! Returns a Matrix that is the outer product of two vectors

        Matrix outerProduct(const Array &vec1, const Array &vec2);
        
        inline Matrix outerProduct(const Array &vec1, const Array &vec2){
            QL_REQUIRE(vec1.size() > 0, 
                    "outerProduct: vec1 must be of positive dimension");
            QL_REQUIRE(vec2.size() > 0, 
                    "outerProduct: vec2 must be of positive dimension");
            Matrix result(vec1.size(),vec2.size());
            for(int i = 0; i < vec1.size(); i++)
                for(int j = 0; j < vec2.size(); j++)
                    result[i][j] = vec1[i]*vec2[j];
            return result;
        }
        
    }

}

#endif
