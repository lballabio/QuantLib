/*
 * Copyright (C) 2000
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
    Revision 1.2  2001/01/05 11:02:38  lballabio
    Renamed SinglePathPricer to PathPricer

    Revision 1.1  2001/01/04 17:31:23  marmar
    Alpha version of the Monte Carlo tools.
            
*/

#include "mcasianpricer.h"
#include "handle.h"
#include "singlepathcontrolvariatedpricer.h"
#include "singlepathaveragepriceasianpricer.h"
#include "singlepathgeometricasianpricer.h"
#include "geometricasianoption.h"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::MonteCarlo1D;
        using MonteCarlo::PathPricer;
        using MonteCarlo::SinglePathControlVariatedPricer;
        using MonteCarlo::SinglePathAveragePriceAsianPricer;
        using MonteCarlo::SinglePathGeometricAsianPricer;

        McAsianPricer::McAsianPricer(Option::Type type, double underlying, 
          double strike, Rate underlyingGrowthRate, Rate riskFreeRate, 
          double residualTime, double volatility, int timesteps, int samples,
          long seed)
        : McPricer(samples, seed) {

            Handle<PathPricer> spPricer(
                new SinglePathAveragePriceAsianPricer(type, underlying, strike,
                    QL_EXP(-riskFreeRate*residualTime)));

            Handle<PathPricer> controlVariateSpPricer(
                new SinglePathGeometricAsianPricer(type, underlying, strike,
                    QL_EXP(-riskFreeRate*residualTime)));

            double controlVariatePrice = GeometricAsianOption(type, underlying,
                strike, underlyingGrowthRate, riskFreeRate, residualTime, 
                volatility).value(); 

            Handle<PathPricer> controlVariatedPricer(
                new SinglePathControlVariatedPricer(spPricer, 
                    controlVariateSpPricer, controlVariatePrice));

            montecarloPricer_ = MonteCarlo1D(controlVariatedPricer, 
                underlyingGrowthRate, riskFreeRate, residualTime, volatility,
                timesteps, seed);
        }

    }

}
