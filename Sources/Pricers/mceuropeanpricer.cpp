
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

/*! \file mceuropeanpricer.cpp

    $Source$
    $Name$
    $Log$
    Revision 1.11  2001/04/09 14:13:34  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.10  2001/04/06 18:46:22  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.9  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.8  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.7  2001/02/13 10:02:57  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.6  2001/02/05 16:54:05  marmar
    McAsianPricer replaced by AveragePriceAsian and AverageStrikeAsian

    Revision 1.5  2001/01/30 15:57:39  marmar
    Now using OneFactorMonteCarloOption

    Revision 1.4  2001/01/29 15:01:25  marmar
    Modified to accomodate code-sharing with
    multi-dimensional Monte Carlo

    Revision 1.3  2001/01/05 11:42:38  lballabio
    Renamed SinglePathEuropeanPricer to EuropeanPathPricer

    Revision 1.2  2001/01/05 11:02:38  lballabio
    Renamed SinglePathPricer to PathPricer

    Revision 1.1  2001/01/04 17:31:23  marmar
    Alpha version of the Monte Carlo tools.

*/

#include "ql/Pricers/mceuropeanpricer.hpp"
#include "ql/handle.hpp"
#include "ql/MonteCarlo/europeanpathpricer.hpp"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::OneFactorMonteCarloOption;
        using MonteCarlo::PathPricer;
        using MonteCarlo::StandardPathGenerator;
        using MonteCarlo::EuropeanPathPricer;

        McEuropeanPricer::McEuropeanPricer(Option::Type type, double underlying,
          double strike, Rate dividendYield, Rate riskFreeRate,
          double residualTime, double volatility, int timesteps, long samples,
          long seed)
        : McPricer(samples, seed) {
            //! Initialize the path generator
            double deltaT = residualTime/timesteps;
            double mu = deltaT * (riskFreeRate - dividendYield
                                    - 0.5 * volatility * volatility);

            double variance = volatility*volatility*deltaT;

            Handle<StandardPathGenerator> pathGenerator(
                    new StandardPathGenerator(timesteps, mu, variance, seed));

            //! Initialize the pricer on the single Path
            Handle<PathPricer> euroPathPricer(new EuropeanPathPricer(type,
                underlying, strike, QL_EXP(-riskFreeRate*residualTime)));

            //! Initialize the one-factor Monte Carlo
            montecarloPricer_ = OneFactorMonteCarloOption(pathGenerator,
                                                     euroPathPricer);
        }

    }

}
