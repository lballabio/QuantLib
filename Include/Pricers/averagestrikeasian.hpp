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

/*! \file averagestrikeasian.hpp
    
    $Source$
    $Name$
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

    Revision 1.2  2001/02/13 10:02:17  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.1  2001/02/05 16:53:38  marmar
    McAsianPricer replaced by AveragePriceAsian and AverageStrikeAsian

*/

#ifndef quantlib_pricers_average_strike_asian_pricer_h
#define quantlib_pricers_average_strike_asian_pricer_h

#include "qldefines.hpp"
#include "options.hpp"
#include "rate.hpp"
#include "MonteCarlo/mcpricer.hpp"

namespace QuantLib {

    namespace Pricers {
        /*! This is an example of Monte Carlo pricer that
            uses a control variate.
        */

        class AverageStrikeAsian: public McPricer {
        public:
            AverageStrikeAsian(Option::Type type, double underlying, 
               double strike, Rate dividendYield, Rate riskFreeRate, 
               double residualTime, double volatility, int timesteps, 
               long samples, long seed = 0);
        };

    }

}


#endif
