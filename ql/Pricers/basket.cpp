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

/*! \file basket.cpp
    \brief simple example of multi-factor Monte Carlo pricer

    \fullpath
    ql/Pricers/%basket.cpp
*/

// $Id$

#include "ql/handle.hpp"
#include "ql/MonteCarlo/basketpathpricer.hpp"
#include "ql/MonteCarlo/mctypedefs.hpp"
#include "ql/Pricers/basket.hpp"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::MultiPathPricer;
        using MonteCarlo::GaussianMultiPathGenerator;
        using MonteCarlo::BasketPathPricer;
        using MonteCarlo::MultiFactorMonteCarloOption;

        Basket::Basket(const Array &underlying,
            const Array &dividendYield, const Math::Matrix &covariance,
            Rate riskFreeRate,  double residualTime,
            unsigned int samples, bool antitheticVariance, long seed) {
            QL_REQUIRE(samples >= 30,
                "Basket: less than 30 samples. Are you joking?");
            QL_REQUIRE(covariance.rows() == covariance.columns(),
                "Basket: covariance matrix not square");
            QL_REQUIRE(covariance.rows() == underlying.size(),
                "Basket: underlying size does not match that of"
                " covariance matrix");
            QL_REQUIRE(covariance.rows() == dividendYield.size(),
                "Basket: dividendYield size does not match"
                " that of covariance matrix");
            QL_REQUIRE(residualTime > 0,
                "Basket: residual time must be positive");

            //! Initialize the path generator
            Array mu(riskFreeRate - dividendYield
                - 0.5 * covariance.diagonal());

            Handle<GaussianMultiPathGenerator> pathGenerator(
                new GaussianMultiPathGenerator(mu, covariance,
                std::vector<Time>(1, residualTime), seed));

            //! Initialize the pricer on the path pricer
            Handle<MultiPathPricer> pathPricer(new BasketPathPricer(
                underlying, QL_EXP(-riskFreeRate*residualTime),
                antitheticVariance));

             //! Initialize the multi-factor Monte Carlo
            mcModel_ = Handle<MonteCarlo::MonteCarloModel<Math::Statistics, MonteCarlo::GaussianMultiPathGenerator, MonteCarlo::MultiPathPricer> > (
                new MonteCarlo::MonteCarloModel<Math::Statistics, MonteCarlo::GaussianMultiPathGenerator, MonteCarlo::MultiPathPricer> (
                                        pathGenerator, pathPricer,
                                        Math::Statistics()));

            mcModel_->addSamples(samples);
        }

    }

}
