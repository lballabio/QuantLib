
/*
 * Copyright (C) 2000
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

/*! \file operatortraits.h
    \brief differential operator traits

    $Source$
    $Name$
    $Log$
    Revision 1.6  2001/01/17 14:37:55  nando
    tabs removed

    Revision 1.5  2001/01/08 11:44:17  lballabio
    Array back into QuantLib namespace - Math namespace broke expression templates, go figure

    Revision 1.4  2001/01/08 10:28:16  lballabio
    Moved Array to Math namespace

    Revision 1.3  2000/12/14 12:32:30  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_operator_traits_h
#define quantlib_operator_traits_h

#include "qldefines.h"
#include "array.h"

namespace QuantLib {

    namespace FiniteDifferences {

        template<class Operator>
        class OperatorTraits {
          public:
            typedef Array arrayType;
        };

    }

}


#endif
