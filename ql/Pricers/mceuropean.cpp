
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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
*/

// $Id$

#include <ql/Pricers/mceuropean.hpp>
#include <ql/MonteCarlo/europeanpathpricer.hpp>

namespace QuantLib {

    namespace Pricers {

        using Math::Statistics;
        using namespace MonteCarlo;

        McEuropean::McEuropean(Option::Type type,
          double underlying, double strike, Spread dividendYield,
          Rate riskFreeRate, double residualTime, double volatility,
          bool antitheticVariance, long seed) {


            //! Initialize the path generator
            double mu = riskFreeRate - dividendYield
                                     - 0.5 * volatility * volatility;

            Handle<GaussianPathGenerator_old> pathGenerator(
                new GaussianPathGenerator_old(mu, volatility*volatility,
                    residualTime, 1, seed));

            //! Initialize the path pricer
            Handle<PathPricer_old<Path> > euroPathPricer(
                new EuropeanPathPricer_old(type,
                underlying, strike, QL_EXP(-riskFreeRate*residualTime),
                antitheticVariance));

            //! Initialize the one-factor Monte Carlo
            mcModel_ = Handle<MonteCarloModel<SingleAsset_old<
                                              PseudoRandom_old> > > (
                new MonteCarloModel<SingleAsset_old<PseudoRandom_old> > (
                pathGenerator, euroPathPricer,
                Statistics(), false));

        }

    }

}
