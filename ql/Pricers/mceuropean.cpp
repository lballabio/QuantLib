

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file mceuropean.cpp
    \brief simple example of Monte Carlo pricer

    \fullpath
    ql/Pricers/%mceuropean.cpp
*/

// $Id$

#include <ql/Pricers/mceuropean.hpp>
#include <ql/MonteCarlo/europeanpathpricer.hpp>

namespace QuantLib {

    namespace Pricers {

        using Math::Statistics;
        using MonteCarlo::Path;
        using MonteCarlo::GaussianPathGenerator;
        using MonteCarlo::PathPricer;
        using MonteCarlo::MonteCarloModel;
        using MonteCarlo::EuropeanPathPricer;

        McEuropean::McEuropean(Option::Type type,
          double underlying, double strike, Spread dividendYield,
          Rate riskFreeRate, double residualTime, double volatility,
          bool antitheticVariance, long seed) {


            //! Initialize the path generator
            double mu = riskFreeRate - dividendYield
                                     - 0.5 * volatility * volatility;

            Handle<GaussianPathGenerator> pathGenerator(
                new GaussianPathGenerator(mu, volatility*volatility,
                    residualTime, 1, seed));

            //! Initialize the pricer on the single Path
            Handle<PathPricer<Path> > euroPathPricer(
                new EuropeanPathPricer(type,
                underlying, strike, QL_EXP(-riskFreeRate*residualTime),
                antitheticVariance));

            //! Initialize the one-factor Monte Carlo
            mcModel_ = Handle<MonteCarloModel<Statistics,
                GaussianPathGenerator, PathPricer<Path> > > (
                new MonteCarloModel<Statistics,
                GaussianPathGenerator, PathPricer<Path> > (
                pathGenerator, euroPathPricer,
                Statistics()));

        }

    }

}
