
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

/*! \file averagestrikeasian.cpp

    $Source$
    $Name$
    $Log$
    Revision 1.7  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.6  2001/04/04 13:32:54  enri
    tons of typos fixed

    Revision 1.5  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.4  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.3  2001/02/13 15:05:24  lballabio
    Trimmed a couple of long file names for Mac compatibility

    Revision 1.2  2001/02/13 10:02:57  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.1  2001/02/05 16:54:06  marmar
    McAsianPricer replaced by AveragePriceAsian and AverageStrikeAsian

*/

#include "handle.hpp"
#include "MonteCarlo/standardpathgenerator.hpp"
#include "Pricers/averagestrikeasian.hpp"
#include "MonteCarlo/controlvariatedpathpricer.hpp"
#include "MonteCarlo/avgstrikeasianpathpricer.hpp"
#include "MonteCarlo/geometricasianpathpricer.hpp"
#include "Pricers/geometricasianoption.hpp"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::OneFactorMonteCarloOption;
        using MonteCarlo::PathPricer;
        using MonteCarlo::StandardPathGenerator;
        using MonteCarlo::ControlVariatedPathPricer;
        using MonteCarlo::AverageStrikeAsianPathPricer;
        using MonteCarlo::GeometricAsianPathPricer;

        AverageStrikeAsian::AverageStrikeAsian(Option::Type type,
          double underlying, double strike, Rate dividendYield,
          Rate riskFreeRate, double residualTime, double volatility,
          int timesteps, long samples, long seed)
        : McPricer(samples, seed) {
            //! Initialize the path generator
            double deltaT = residualTime/timesteps;
            double mu = deltaT * (riskFreeRate - dividendYield
                                    - 0.5 * volatility * volatility);
            double variance = volatility*volatility*deltaT;

            Handle<StandardPathGenerator> pathGenerator(
                    new StandardPathGenerator(timesteps, mu, variance, seed));

            //! Initialize the pricer on the single Path
            Handle<PathPricer> spPricer(
                new AverageStrikeAsianPathPricer(type, underlying, strike,
                    QL_EXP(-riskFreeRate*residualTime)));

            Handle<PathPricer> controlVariateSpPricer(
                new GeometricAsianPathPricer(type, underlying, strike,
                    QL_EXP(-riskFreeRate*residualTime)));

            double controlVariatePrice = GeometricAsianOption(type, underlying,
                strike, dividendYield, riskFreeRate, residualTime,
                volatility).value();

            Handle<PathPricer> controlVariatedPricer(
                new ControlVariatedPathPricer(spPricer,
                    controlVariateSpPricer, controlVariatePrice));

            //! Initialize the one-dimensional Monte Carlo
            montecarloPricer_ = OneFactorMonteCarloOption(pathGenerator,
                                     controlVariatedPricer);
        }

    }

}
