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

/*! \file mcmaxbasket.cpp
    \brief Max Basket Monte Carlo pricer

    \fullpath
    ql/Pricers/%mcmaxbasket.cpp
*/

// $Id$

#include <ql/handle.hpp>
#include <ql/MonteCarlo/maxbasketpathpricer.hpp>
#include <ql/MonteCarlo/mctypedefs.hpp>
#include <ql/Pricers/mcmaxbasket.hpp>

namespace QuantLib {

    namespace Pricers {

        using Math::Statistics;
        using Math::Matrix;
        using MonteCarlo::MultiPath;
        using MonteCarlo::GaussianMultiPathGenerator;
        using MonteCarlo::PathPricer;
        using MonteCarlo::MonteCarloModel;
        using MonteCarlo::MaxBasketPathPricer;

        McMaxBasket::McMaxBasket(const Array& underlying,
            const Array& dividendYield, const Math::Matrix& covariance,
            Rate riskFreeRate,  double residualTime,
            bool antitheticVariance, long seed) {

            QL_REQUIRE(covariance.rows() == covariance.columns(),
                "McMaxBasket: covariance matrix not square");
            QL_REQUIRE(covariance.rows() == underlying.size(),
                "McMaxBasket: underlying size does not match that of"
                " covariance matrix");
            QL_REQUIRE(covariance.rows() == dividendYield.size(),
                "McMaxBasket: dividendYield size does not match"
                " that of covariance matrix");
            QL_REQUIRE(residualTime > 0,
                "McMaxBasket: residual time must be positive");

            //! Initialize the path generator
            Array mu(riskFreeRate - dividendYield
                - 0.5 * covariance.diagonal());

            Handle<GaussianMultiPathGenerator> pathGenerator(
                new GaussianMultiPathGenerator(mu, covariance,
                std::vector<Time>(1, residualTime), seed));

            //! Initialize the pricer on the path pricer
            Handle<PathPricer<MultiPath> > pathPricer(
                new MaxBasketPathPricer(
                underlying, QL_EXP(-riskFreeRate*residualTime),
                antitheticVariance));

             //! Initialize the multi-factor Monte Carlo
            mcModel_ = Handle<MonteCarlo::MonteCarloModel<Statistics,
                GaussianMultiPathGenerator,
                PathPricer<MultiPath> > > (
                new MonteCarloModel<Statistics,
                GaussianMultiPathGenerator,
                PathPricer<MultiPath> > (pathGenerator,
                pathPricer, Statistics()));

        }

    }

}
