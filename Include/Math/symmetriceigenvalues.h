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

/*! \file eigenvalues.h
    \brief Gives the eigenvalues and eigenvectors of a real symmetric matrix

    $Source$
    $Log$
    Revision 1.1  2001/03/28 07:40:58  marmar
    Functions to calculate eigenvalues and eigenvectors of a symmetric matrix added

*/

#ifndef quantlib_symmetric_eigenvalues_h
#define quantlib_symmetric_eigenvalues_h

#include "symmetricschurdecomposition.h"

namespace QuantLib {

    namespace Math {
        
        inline Array SymmetricEigenvalues(Matrix &s){
            return SymmetricSchurDecomposition(s).eigenvalues();
        }

        inline Matrix SymmetricEigenvectors(Matrix &s){
            return SymmetricSchurDecomposition(s).eigenvectors();
        }

    }

}

#endif


