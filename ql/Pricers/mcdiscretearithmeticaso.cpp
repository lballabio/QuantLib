

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
/*! \file mcdiscretearithmeticaso.cpp
    \brief Discrete Arithmetic Average Strike Option

    \fullpath
    ql/Pricers/%mcdiscretearithmeticaso.cpp
*/

// $Id$

#include <ql/Pricers/mcdiscretearithmeticaso.hpp>
#include <ql/Pricers/discretegeometricaso.hpp>
#include <ql/MonteCarlo/arithmeticasopathpricer.hpp>
#include <ql/MonteCarlo/geometricasopathpricer.hpp>

namespace QuantLib {

    namespace Pricers {

        using Math::Statistics;
        using MonteCarlo::Path;
        using MonteCarlo::GaussianPathGenerator;
        using MonteCarlo::PathPricer;
        using MonteCarlo::MonteCarloModel;
        using MonteCarlo::ArithmeticASOPathPricer;
        using MonteCarlo::GeometricASOPathPricer;

        McDiscreteArithmeticASO::McDiscreteArithmeticASO(Option::Type type,
          double underlying,
          Spread dividendYield, Rate riskFreeRate,
          const std::vector<Time>& times, double volatility,
          bool antitheticVariance, bool controlVariate, long seed) {

            QL_REQUIRE(times.size() >= 2,
                "McDiscreteArithmeticASO: you must have at least 2 time-steps");
            //! Initialize the path generator
            double mu = riskFreeRate - dividendYield
                                     - 0.5 * volatility * volatility;

            Handle<GaussianPathGenerator> pathGenerator(
                new GaussianPathGenerator(mu, volatility*volatility,
                    times, seed));

            //! Initialize the Path Pricer
            Handle<PathPricer<Path> > spPricer(
                new ArithmeticASOPathPricer(type, underlying,
                    QL_EXP(-riskFreeRate*times.back()), antitheticVariance));

            if (controlVariate) {
                Handle<PathPricer<Path> > controlVariateSpPricer(
                    new GeometricASOPathPricer(type, underlying,
                    QL_EXP(-riskFreeRate*times.back()),
                    antitheticVariance));

                double controlVariatePrice = DiscreteGeometricASO(type,
                    underlying, dividendYield, riskFreeRate,
                    times, volatility).value();

                //! Initialize the one-dimensional Monte Carlo
                mcModel_ = Handle<MonteCarloModel<Statistics,
                    GaussianPathGenerator,
                    PathPricer<Path> > > (
                    new MonteCarloModel<Statistics,
                    GaussianPathGenerator,
                    PathPricer<Path> > (pathGenerator, spPricer,
                    Statistics(),
                    controlVariateSpPricer, controlVariatePrice));
            } else {
                //! Initialize the one-dimensional Monte Carlo
                mcModel_ = Handle<MonteCarloModel<Statistics,
                    GaussianPathGenerator,
                    PathPricer<Path> > > (
                    new MonteCarloModel<Statistics,
                    GaussianPathGenerator,
                    PathPricer<Path> > (pathGenerator, spPricer,
                    Statistics()));
            }

        }

    }

}
