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
/*! \file everestoption.hpp
    
    $Source$
    $Log$
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

#include "qldefines.hpp"
#include "rate.hpp"
#include "date.hpp"
#include "MonteCarlo/multifactorpricer.hpp"

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
