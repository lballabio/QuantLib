
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file symmetriceigenvalues.hpp
    \brief Gives the eigenvalues and eigenvectors of a real symmetric matrix

    $Source$
    $Log$
    Revision 1.1  2001/04/09 14:05:48  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.3  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.2  2001/04/06 16:19:12  lballabio
    Fixed Doxygen-related stuff

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.1  2001/03/28 07:40:58  marmar
    Functions to calculate eigenvalues and eigenvectors of a symmetric matrix added

*/

#ifndef quantlib_symmetric_eigenvalues_h
#define quantlib_symmetric_eigenvalues_h

#include "ql/Math/symmetricschurdecomposition.hpp"

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


