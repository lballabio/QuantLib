
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

/*! \file averagepriceasian.hpp

    $Source$
    $Name$
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

    Revision 1.2  2001/02/13 10:02:17  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.1  2001/02/05 16:53:38  marmar
    McAsianPricer replaced by AveragePriceAsian and AverageStrikeAsian

    Revision 1.5  2001/01/29 15:02:13  marmar
    Modified to accomodate code-sharing with
    multi-dimensional Monte Carlo

    Revision 1.4  2001/01/17 11:54:18  marmar
    Some documentation added and 80 columns format enforced.

    Revision 1.3  2001/01/05 11:42:38  lballabio
    Renamed SinglePathEuropeanPricer to EuropeanPathPricer

    Revision 1.2  2001/01/05 11:02:38  lballabio
    Renamed SinglePathPricer to PathPricer

    Revision 1.1  2001/01/04 17:31:23  marmar
    Alpha version of the Monte Carlo tools.

*/

#ifndef quantlib_pricers_average_price_asian_h
#define quantlib_pricers_average_price_asian_h

#include "qldefines.hpp"
#include "options.hpp"
#include "rate.hpp"
#include "MonteCarlo/mcpricer.hpp"

namespace QuantLib {

    namespace Pricers {
        /*! This is an example of Monte Carlo pricer that
            uses a control variate.
        */

        class AveragePriceAsian: public McPricer {
        public:
            AveragePriceAsian(Option::Type type, double underlying,
                double strike, Rate dividendYield,   Rate riskFreeRate,
                double residualTime, double volatility, int timesteps,
                long samples, long seed=0);
        };

    }

}


#endif
