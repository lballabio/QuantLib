
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
/*! \file pagodaoption.hpp

    $Source$
    $Log$
    Revision 1.3  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.2  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.1  2001/04/04 11:07:23  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.1  2001/03/22 12:04:08  marmar
    Introducing pagoda options

*/

#ifndef quantlib_pagoda_option_pricer_h
#define quantlib_pagoda_option_pricer_h

#include "qldefines.hpp"
#include "rate.hpp"
#include "MonteCarlo/multifactorpricer.hpp"

namespace QuantLib {

    namespace Pricers {
        /*! \brief A pagoda option is a roofed asian option.
        Given a certain portfolio, of assets at the end of the period
        it is returned the minimum of a given roof and a certain fraction of
        the positive portfolio performance.
        If the performance of the portfolio is below then option is null.
        */

        class PagodaOption: public MultiFactorPricer {
          public:
            PagodaOption(const Array &portfolio,
                         double fraction,
                         double roof,
                         double residualTime,
                         const Math::Matrix &covariance,
                         const Array &dividendYield,
                         Rate riskFreeRate,
                         int timesteps,
                         long samples,
                         long seed = 0);
        };

    }

}

#endif
