
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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
        using MonteCarlo::GaussianPathGenerator;
        using MonteCarlo::PathPricer;
        using MonteCarlo::MonteCarloModel;
        using MonteCarlo::ArithmeticAPOPathPricer;
        using MonteCarlo::GeometricAPOPathPricer;

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

            Handle<GaussianPathGenerator> pathGenerator(
                new GaussianPathGenerator(mu, volatility*volatility,
                    times, seed));


            //! Initialize the Path Pricer
            Handle<PathPricer<Path> > spPricer(
                new ArithmeticAPOPathPricer(type, underlying, strike,
                    QL_EXP(-riskFreeRate*times.back()), antitheticVariance));


            if (controlVariate) {
                Handle<PathPricer<Path> > controlVariateSpPricer(
                    new GeometricAPOPathPricer(type, underlying, strike,
                        QL_EXP(-riskFreeRate*times.back()), antitheticVariance));

                double controlVariatePrice = DiscreteGeometricAPO(type,
                    underlying, strike, dividendYield, riskFreeRate,
                    times, volatility).value();

                //! Initialize the Monte Carlo model
                mcModel_ = Handle<MonteCarloModel<Statistics,
                    GaussianPathGenerator, PathPricer<Path> > > (
                    new MonteCarloModel<Statistics,
                    GaussianPathGenerator, PathPricer<Path> > (
                    pathGenerator, spPricer, Statistics(),
                    controlVariateSpPricer, controlVariatePrice));
            } else {
                //! Initialize the Monte Carlo model
                mcModel_ = Handle<MonteCarloModel<Statistics,
                    GaussianPathGenerator, PathPricer<Path> > > (
                    new MonteCarloModel<Statistics,
                    GaussianPathGenerator, PathPricer<Path> > (
                    pathGenerator, spPricer, Statistics()));
            }

        }

    }

}
