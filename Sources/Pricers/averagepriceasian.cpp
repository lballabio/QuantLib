
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

/*! \file averagepriceasian.cpp

    $Source$
    $Name$
    $Log$
    Revision 1.9  2001/05/23 19:30:27  nando
    smoothing #include xx.hpp

    Revision 1.8  2001/04/09 14:13:34  nando
    all the *.hpp moved below the Include/ql level

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

*/

#include "ql/MonteCarlo/standardpathgenerator.hpp"
#include "ql/Pricers/averagepriceasian.hpp"
#include "ql/MonteCarlo/controlvariatedpathpricer.hpp"
#include "ql/MonteCarlo/avgpriceasianpathpricer.hpp"
#include "ql/MonteCarlo/geometricasianpathpricer.hpp"
#include "ql/Pricers/geometricasianoption.hpp"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::OneFactorMonteCarloOption;
        using MonteCarlo::PathPricer;
        using MonteCarlo::StandardPathGenerator;
        using MonteCarlo::ControlVariatedPathPricer;
        using MonteCarlo::AveragePriceAsianPathPricer;
        using MonteCarlo::GeometricAsianPathPricer;

        AveragePriceAsian::AveragePriceAsian(Option::Type type, double underlying,
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
            Handle<PathPricer> spPricer(
                new AveragePriceAsianPathPricer(type, underlying, strike,
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
