/*
 * Copyright (C) 2000, 2001
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

/*! \file mcasianpricer.cpp
    
    $Source$
    $Name$
    $Log$
    Revision 1.7  2001/01/29 15:01:26  marmar
    Modified to accomodate code-sharing with
    multi-dimensional Monte Carlo

    Revision 1.6  2001/01/05 12:28:15  lballabio
    Renamed SinglePathControlVariatedPricer to ControlVariatedPathPricer

    Revision 1.5  2001/01/05 11:52:12  lballabio
    Renamed SinglePathAveragePriceAsianPricer to AverageAsianPathPricer

    Revision 1.4  2001/01/05 11:42:38  lballabio
    Renamed SinglePathEuropeanPricer to EuropeanPathPricer

    Revision 1.3  2001/01/05 11:18:04  lballabio
    Renamed SinglePathGeometricAsianPricer to GeometricAsianPathPricer

    Revision 1.2  2001/01/05 11:02:38  lballabio
    Renamed SinglePathPricer to PathPricer

    Revision 1.1  2001/01/04 17:31:23  marmar
    Alpha version of the Monte Carlo tools.
            
*/

#include "standardpathgenerator.h"
#include "mcasianpricer.h"
#include "handle.h"
#include "controlvariatedpathpricer.h"
#include "averageasianpathpricer.h"
#include "geometricasianpathpricer.h"
#include "geometricasianoption.h"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::MonteCarlo1D;
        using MonteCarlo::PathPricer;
        using MonteCarlo::StandardPathGenerator;
        using MonteCarlo::ControlVariatedPathPricer;
        using MonteCarlo::AverageAsianPathPricer;
        using MonteCarlo::GeometricAsianPathPricer;

        McAsianPricer::McAsianPricer(Option::Type type, double underlying, 
          double strike, Rate underlyingGrowthRate, Rate riskFreeRate, 
          double residualTime, double volatility, int timesteps, long samples,
          long seed)
        : McPricer(samples, seed) {
            //! Initialize the path generator
            double deltaT = residualTime/timesteps;
            double mu = deltaT * (riskFreeRate - underlyingGrowthRate
                                    - 0.5 * volatility * volatility);
            double sigma = volatility*QL_SQRT(deltaT);

            Handle<StandardPathGenerator> pathGenerator(
                    new StandardPathGenerator(timesteps, mu, sigma, seed));
            
            //! Initialize the pricer on the single Path 
            Handle<PathPricer> spPricer(
                new AverageAsianPathPricer(type, underlying, strike,
                    QL_EXP(-riskFreeRate*residualTime)));

            Handle<PathPricer> controlVariateSpPricer(
                new GeometricAsianPathPricer(type, underlying, strike,
                    QL_EXP(-riskFreeRate*residualTime)));

            double controlVariatePrice = GeometricAsianOption(type, underlying,
                strike, underlyingGrowthRate, riskFreeRate, residualTime, 
                volatility).value(); 

            Handle<PathPricer> controlVariatedPricer(
                new ControlVariatedPathPricer(spPricer, 
                    controlVariateSpPricer, controlVariatePrice));

            //! Initialize the one-dimensional Monte Carlo
            montecarloPricer_ = MonteCarlo1D(pathGenerator,
                                     controlVariatedPricer);
        }

    }

}
