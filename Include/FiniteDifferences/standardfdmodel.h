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
 *  http://quantlib.sourceforge.net/LICENSE.TXT
*/
/*! \file standardfdmodel.h
    \brief Defines the standard finite-differences model

    $Source$
    $Log$
    Revision 1.1  2001/03/01 13:51:15  marmar
    Choice made for the standard finite-difference model

*/

#ifndef quantlib_standard_finite_differences_model_h
#define quantlib_standard_finite_differences_model_h

#include "finitedifferencemodel.h"
#include "cranknicolson.h"

namespace QuantLib {

    namespace FiniteDifferences {

    /*! \typedef StandardPathGenerator
        The following is the default choice for the finite-difference
        model.
    */
        typedef FiniteDifferenceModel< 
                            CrankNicolson<TridiagonalOperator>  >
                                        StandardFiniteDifferenceModel;
    }
}

#endif
