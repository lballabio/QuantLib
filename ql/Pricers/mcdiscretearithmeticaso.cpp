
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
