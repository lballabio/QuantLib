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

/*! \file averagestrikeasian.cpp
    
    $Source$
    $Name$
    $Log$
    Revision 1.3  2001/02/13 15:05:24  lballabio
    Trimmed a couple of long file names for Mac compatibility

    Revision 1.2  2001/02/13 10:02:57  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.1  2001/02/05 16:54:06  marmar
    McAsianPricer replaced by AveragePriceAsian and AverageStrikeAsian

*/

#include "handle.h"
#include "standardpathgenerator.h"
#include "averagestrikeasian.h"
#include "controlvariatedpathpricer.h"
#include "avgstrikeasianpathpricer.h"
#include "geometricasianpathpricer.h"
#include "geometricasianoption.h"

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
