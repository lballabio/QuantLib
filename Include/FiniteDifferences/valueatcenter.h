
/*
 * Copyright (C) 2000, 2001
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

/*! \file valueatcenter.h
    \brief compute value, first, and second derivatives at grid center

    $Source$
    $Log$
    Revision 1.1  2001/03/21 10:47:08  marmar
    valueAtCenter, firstDerivativeAtCenter, secondDerivativeAtCenter,
    are no longer methods of BSMNumericalOption but separate
    functions


*/

#ifndef quantlib_finite_difference_value_at_center_h
#define quantlib_finite_difference_value_at_center_h

#include "qldefines.h"
#include "array.h"

namespace QuantLib {

    namespace FiniteDifferences {

        double valueAtCenter(const Array& a);

        double firstDerivativeAtCenter(const Array& a, const Array& g);

        double secondDerivativeAtCenter(const Array& a, const Array& g);        
        
    }

}


#endif
