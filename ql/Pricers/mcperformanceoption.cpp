

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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
/*! \file mcperformanceoption.cpp
    \brief Performance option priced with Monte Carlo simulation

    \fullpath
    ql/Pricers/%mcperformanceoption.cpp
*/

// $Id$

#include <ql/Pricers/mcperformanceoption.hpp>
#include <ql/MonteCarlo/performanceoptionpathpricer.hpp>

namespace QuantLib {

    namespace Pricers {

        using Math::Statistics;
        using MonteCarlo::Path;
        using MonteCarlo::GaussianPathGenerator;
        using MonteCarlo::PathPricer;
        using MonteCarlo::MonteCarloModel;
        using MonteCarlo::PerformanceOptionPathPricer;

        McPerformanceOption::McPerformanceOption(Option::Type type,
          double underlying, double moneyness,
          const std::vector<Spread>& dividendYield,
          const std::vector<Rate>& riskFreeRate,
          const std::vector<Time>& times,
          const std::vector<double>& volatility,
          bool antitheticVariance, long seed) {

            Size dimension = times.size();
            QL_REQUIRE(dividendYield.size()==dimension,
                "McPerformanceOption: dividendYield vector of wrong size");
            QL_REQUIRE(riskFreeRate.size()==dimension,
                "McPerformanceOption: riskFreeRate vector of wrong size");
            QL_REQUIRE(volatility.size()==dimension,
                "McPerformanceOption: volatility vector of wrong size");

            //! Initialize the path generator
            std::vector<double> mu(dimension);
            std::vector<double> diffusion(dimension);
            std::vector<double> discounts(dimension);
            for (Size i = 0; i<dimension; i++) {
                mu[i]= riskFreeRate[i] - dividendYield[i] -
                    0.5 * volatility[i] * volatility[i];
                diffusion[i]= volatility[i] * volatility[i];
                if (i==0)
                    discounts[i] = QL_EXP(-riskFreeRate[i]*times[i]);
                else
                    discounts[i] = discounts[i-1]*
                        QL_EXP(-riskFreeRate[i]*(times[i]-times[i-1]));
            }


            Handle<GaussianPathGenerator> pathGenerator(
                new GaussianPathGenerator(mu, diffusion,
                    times, seed));

            //! Initialize the pricer on the single Path
            Handle<PathPricer<Path> > performancePathPricer(
                new PerformanceOptionPathPricer(type,
                underlying, moneyness, discounts,
                antitheticVariance));

            //! Initialize the one-factor Monte Carlo
            mcModel_ = Handle<MonteCarloModel<Statistics,
                GaussianPathGenerator, PathPricer<Path> > > (
                new MonteCarloModel<Statistics,
                GaussianPathGenerator, PathPricer<Path> > (
                pathGenerator, performancePathPricer,
                Statistics()));

        }

    }

}
