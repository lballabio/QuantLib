

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
/*! \file mcdiscretearithmeticapo.cpp
    \brief Discrete Arithmetic Average Price Option

    \fullpath
    ql/Pricers/%mcdiscretearithmeticapo.cpp
*/

// $Id$

#include <ql/Pricers/mcdiscretearithmeticapo.hpp>
#include <ql/Pricers/discretegeometricapo.hpp>
#include <ql/MonteCarlo/arithmeticapopathpricer.hpp>
#include <ql/MonteCarlo/geometricapopathpricer.hpp>

namespace QuantLib {

    namespace Pricers {

        using Math::Statistics;
        using MonteCarlo::Path;
        using MonteCarlo::GaussianPathGenerator_old;
        using MonteCarlo::PathPricer_old;
        using MonteCarlo::MonteCarloModel;
        using MonteCarlo::ArithmeticAPOPathPricer_old;
        using MonteCarlo::GeometricAPOPathPricer_old;

        McDiscreteArithmeticAPO::McDiscreteArithmeticAPO(Option::Type type,
            double underlying, double strike,
            Spread dividendYield, Rate riskFreeRate,
            const std::vector<Time>& times, double volatility,
            bool antitheticVariance, bool controlVariate, long seed) {

            QL_REQUIRE(times.size() >= 2,
                "McDiscreteArithmeticAPO: you must have at least 2 time-steps");
            //! Initialize the path generator
            double mu = riskFreeRate - dividendYield
                                     - 0.5 * volatility * volatility;

            Handle<GaussianPathGenerator_old> pathGenerator(
                new GaussianPathGenerator_old(mu, volatility*volatility,
                    TimeGrid(times.begin(), times.end()),
                    seed));


            //! Initialize the Path Pricer
            Handle<PathPricer_old<Path> > spPricer(
                new ArithmeticAPOPathPricer_old(type, underlying, strike,
                    QL_EXP(-riskFreeRate*times.back()), antitheticVariance));


            if (controlVariate) {
                Handle<PathPricer_old<Path> > controlVariateSpPricer(
                    new GeometricAPOPathPricer_old(type, underlying, strike,
                        QL_EXP(-riskFreeRate*times.back()), antitheticVariance));

                double controlVariatePrice = DiscreteGeometricAPO(type,
                    underlying, strike, dividendYield, riskFreeRate,
                    times, volatility).value();

                //! Initialize the Monte Carlo model
                mcModel_ = Handle<MonteCarloModel<Statistics,
                    GaussianPathGenerator_old, PathPricer_old<Path> > > (
                    new MonteCarloModel<Statistics,
                    GaussianPathGenerator_old, PathPricer_old<Path> > (
                    pathGenerator, spPricer, Statistics(),
                    controlVariateSpPricer, controlVariatePrice));
            } else {
                //! Initialize the Monte Carlo model
                mcModel_ = Handle<MonteCarloModel<Statistics,
                    GaussianPathGenerator_old, PathPricer_old<Path> > > (
                    new MonteCarloModel<Statistics,
                    GaussianPathGenerator_old, PathPricer_old<Path> > (
                    pathGenerator, spPricer, Statistics()));
            }

        }

    }

}
