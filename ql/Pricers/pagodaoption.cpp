
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

/*! \file pagodaoption.cpp
    \brief roofed Asian option

    \fullpath
    ql/Pricers/%pagodaoption.cpp
*/

// $Id$

#include "ql/handle.hpp"
#include "ql/Pricers/pagodaoption.hpp"
#include "ql/MonteCarlo/pagodapathpricer.hpp"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::MultiPathPricer;
        using MonteCarlo::GaussianMultiPathGenerator;
        using MonteCarlo::PagodaPathPricer;
        using MonteCarlo::MultiFactorMonteCarloOption;

        PagodaOption::PagodaOption(const Array& portfolio,
            double fraction, double roof,
            const Array& dividendYield,
            const Math::Matrix& covariance,
            Rate riskFreeRate,
            const std::vector<Time>& times,
            long samples, bool antithetic, long seed)
        : MultiFactorPricer(samples, seed){
            QL_REQUIRE(covariance.rows() == covariance.columns(),
                "PagodaOption: covariance matrix not square");
            QL_REQUIRE(covariance.rows() == portfolio.size(),
                "PagodaOption: underlying size does not match that of"
                " covariance matrix");
            QL_REQUIRE(covariance.rows() == dividendYield.size(),
                "PagodaOption: dividendYield size does not match"
                " that of covariance matrix");
            QL_REQUIRE(fraction > 0,
                "PagodaOption: option fraction must be positive");
            QL_REQUIRE(roof > 0,
                "PagodaOption: roof must be positive");
            QL_REQUIRE(times.size() >= 1,
                "PagodaOption: you must have at least one time-step");

            //! Initialize the path generator
            Array mu(riskFreeRate - dividendYield
                            - 0.5 * covariance.diagonal());

            Handle<GaussianMultiPathGenerator> pathGenerator(
                new GaussianMultiPathGenerator(mu,
                                               covariance,
                                               times,
                                               seed));
            double residualTime = times[times.size()-1];

            //! Initialize the pricer on the path pricer
            Handle<MultiPathPricer> pathPricer(
                new PagodaPathPricer(portfolio, roof,
                        fraction * QL_EXP(-riskFreeRate*residualTime),
                        antithetic));

             //! Initialize the multi-factor Monte Carlo
            montecarloPricer_ = Handle<MultiFactorMonteCarloOption>(
                                        new MultiFactorMonteCarloOption(
                                        pathGenerator, pathPricer,
                                        Math::Statistics()));
        }

    }

}
