
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

/*! \file dminus.hpp
    \brief \f$ D_{-} \f$ matricial representation

    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.2  2001/01/17 14:37:55  nando
    tabs removed

    Revision 1.1  2001/01/04 11:57:07  lballabio
    Differential operators added

*/

#ifndef QUANTLIB_D_MINUS_H
#define QUANTLIB_D_MINUS_H

#include "qldefines.hpp"
#include "FiniteDifferences/tridiagonaloperator.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        //! \f$ D_{-} \f$ matricial representation
        /*! The differential operator \f$ D_{-} \f$ discretizes the
            first derivative with the first-order formula
            \f[ \frac{\partial u_{i}}{\partial x} =
                \frac{u_{i}-u_{i-1}}{h} \equiv D_{-} u_{i}
            \f]
        */
        class DMinus : public TridiagonalOperator {
          public:
            DMinus() {}
            DMinus(int gridPoints, double h);
        };

        // inline definitions

        inline DMinus::DMinus(int gridPoints, double h)
        : TridiagonalOperator(gridPoints) {
            setFirstRow(-1/h,1/h);                  // linear extrapolation
            setMidRows(-1/h,1/h,0.0);
            setLastRow(-1/h,1/h);
        }

    }

}


#endif
