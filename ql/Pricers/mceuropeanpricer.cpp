
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

/*! \file mceuropeanpricer.cpp
    \brief simple example of Monte Carlo pricer

    \fullpath
    Pricers/%mceuropeanpricer.cpp
*/

// $Id$

#include "ql/Pricers/mceuropeanpricer.hpp"
#include "ql/handle.hpp"
#include "ql/MonteCarlo/europeanpathpricer.hpp"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::OneFactorMonteCarloOption;
        using MonteCarlo::PathPricer;
        using MonteCarlo::GaussianPathGenerator;
        using MonteCarlo::EuropeanPathPricer;

        McEuropeanPricer::McEuropeanPricer(Option::Type type, 
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
            Handle<PathPricer> euroPathPricer(new EuropeanPathPricer(type,
                underlying, strike, QL_EXP(-riskFreeRate*residualTime)));

            //! Initialize the one-factor Monte Carlo
            montecarloPricer_ = Handle<OneFactorMonteCarloOption>(
                new OneFactorMonteCarloOption(
                    pathGenerator, euroPathPricer,
                    Math::Statistics()));
        }

    }

}
