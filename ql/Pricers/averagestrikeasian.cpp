
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

/*! \file averagestrikeasian.cpp
    \brief example of Monte Carlo pricer using a control variate

    \fullpath
    ql/Pricers/%averagestrikeasian.cpp
*/

// $Id$

#include "ql/MonteCarlo/gaussianpathgenerator.hpp"
#include "ql/Pricers/averagestrikeasian.hpp"
#include "ql/MonteCarlo/controlvariatedpathpricer.hpp"
#include "ql/MonteCarlo/avgstrikeasianpathpricer.hpp"
#include "ql/MonteCarlo/geometricasianpathpricer.hpp"
#include "ql/Pricers/geometricasianoption.hpp"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::OneFactorMonteCarloOption;
        using MonteCarlo::PathPricer;
        using MonteCarlo::GaussianPathGenerator;
        using MonteCarlo::ControlVariatedPathPricer;
        using MonteCarlo::AverageStrikeAsianPathPricer;
        using MonteCarlo::GeometricAsianPathPricer;

        AverageStrikeAsian::AverageStrikeAsian(Option::Type type,
          double underlying, double strike, Rate dividendYield,
          Rate riskFreeRate, double residualTime, double volatility,
          int timesteps, long samples, long seed)
        : McPricer(samples, seed) {
            //! Initialize the path generator
            double mu = riskFreeRate - dividendYield
                                     - 0.5 * volatility * volatility;

            Handle<GaussianPathGenerator> pathGenerator(
                new GaussianPathGenerator(mu, volatility*volatility, 
                    residualTime, timesteps, seed));

            //! Initialize the pricer on the single Path
            Handle<PathPricer> spPricer(
                new AverageStrikeAsianPathPricer(type, underlying, strike,
                    QL_EXP(-riskFreeRate*residualTime)));

            Handle<PathPricer> controlVariateSpPricer(
                new GeometricAsianPathPricer(type, underlying, strike,
                    QL_EXP(-riskFreeRate*residualTime)));

            double controlVariatePrice = GeometricAsianOption(type, 
                underlying, strike, dividendYield, riskFreeRate, 
                residualTime, volatility).value();

            Handle<PathPricer> controlVariatedPricer(
                new ControlVariatedPathPricer(spPricer,
                    controlVariateSpPricer, controlVariatePrice));

            //! Initialize the one-dimensional Monte Carlo
            montecarloPricer_ = Handle<OneFactorMonteCarloOption>(
                new OneFactorMonteCarloOption(
                pathGenerator, controlVariatedPricer,
                Math::Statistics()));
        }

    }

}
