
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
/*! \file everestoption.hpp

    $Source$
    $Log$
    Revision 1.2  2001/05/23 19:30:27  nando
    smoothing #include xx.hpp

    Revision 1.1  2001/04/09 14:05:49  nando
    all the *.hpp moved below the Include/ql level

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

    Revision 1.2  2001/03/07 09:32:34  marmar
    Spot prices not necessary for evaluation of everest option

    Revision 1.1  2001/03/06 16:59:07  marmar
    First, simplified version, of everest option

*/

#ifndef quantlib_pricers_everest_option_h
#define quantlib_pricers_everest_option_h

#include "ql/rate.hpp"
#include "ql/date.hpp"
#include "ql/MonteCarlo/multifactorpricer.hpp"

namespace QuantLib {

    namespace Pricers {

        //! \brief  The everest-type option pricer.
        /*!
        The payoff of an everest option is simply given by the
        final price initial price ratio of the worst performer
        */
        class EverestOption: public MultiFactorPricer {
        public:
            EverestOption(const Array &dividendYield,
                const Math::Matrix &covariance,
                Rate riskFreeRate, Time residualTime,
                long samples, long seed = 0);
        };

    }

}

#endif
