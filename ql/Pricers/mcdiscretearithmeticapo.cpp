
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file mcdiscretearithmeticapo.cpp
    \brief Discrete Arithmetic Average Price Option

    \fullpath
    ql/Pricers/%mcdiscretearithmeticapo.cpp
*/

// $Id$

#include <ql/Pricers/mcdiscretearithmeticapo.hpp>
#include <ql/Pricers/discretegeometricapo.hpp>
#include <ql/MonteCarlo/mctypedefs.hpp>
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
