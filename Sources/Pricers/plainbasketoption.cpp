
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file plainbasketoption.cpp
    \brief simple example of multi-factor Monte Carlo pricer

    $Id$
*/

// $Source$
// $Log$
// Revision 1.13  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.12  2001/07/13 15:25:13  marmar
// MonteCarlo interface changed
//
// Revision 1.11  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.10  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/handle.hpp"
#include "ql/MonteCarlo/basketpathpricer.hpp"
#include "ql/Pricers/plainbasketoption.hpp"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::MultiPathPricer;
        using MonteCarlo::StandardMultiPathGenerator;
        using MonteCarlo::BasketPathPricer;
        using MonteCarlo::MultiFactorMonteCarloOption;

        PlainBasketOption::PlainBasketOption(const Array &underlying,
            const Array &dividendYield, const Math::Matrix &covariance,
            Rate riskFreeRate,  double residualTime, int timesteps,
            long samples, long seed)
        : MultiFactorPricer(samples, seed){
            QL_REQUIRE(covariance.rows() == covariance.columns(),
                "PlainBasketOption: covariance matrix not square");
            QL_REQUIRE(covariance.rows() == underlying.size(),
                "PlainBasketOption: underlying size does not match that of"
                " covariance matrix");
            QL_REQUIRE(covariance.rows() == dividendYield.size(),
                "PlainBasketOption: dividendYield size does not match"
                " that of covariance matrix");
            QL_REQUIRE(residualTime > 0,
                "PlainBasketOption: residual time must be positive");
            QL_REQUIRE(timesteps > 0,
                "PlainBasketOption: time steps must be positive");

            //! Initialize the path generator
            double deltaT = residualTime/timesteps;
            Array mu(deltaT * (riskFreeRate - dividendYield
                                    - 0.5 * covariance.diagonal()));

            Handle<StandardMultiPathGenerator> pathGenerator(
                new StandardMultiPathGenerator(timesteps,
                                               covariance*deltaT,
                                               mu,
                                               seed));

            //! Initialize the pricer on the path pricer
            Handle<MultiPathPricer> pathPricer(new BasketPathPricer(
                underlying, QL_EXP(-riskFreeRate*residualTime)));

             //! Initialize the multi-factor Monte Carlo
            montecarloPricer_ = MultiFactorMonteCarloOption(
                                        pathGenerator, pathPricer,
                                        Math::Statistics());
        }

    }

}
