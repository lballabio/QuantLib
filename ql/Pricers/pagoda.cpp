
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

/*! \file pagoda.cpp
    \brief roofed Asian option

    \fullpath
    ql/Pricers/%pagoda.cpp
*/

// $Id$

#include "ql/handle.hpp"
#include "ql/Pricers/pagoda.hpp"
#include "ql/MonteCarlo/pagodapathpricer.hpp"
#include "ql/MonteCarlo/mctypedefs.hpp"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::MultiPathPricer;
        using MonteCarlo::GaussianMultiPathGenerator;
        using MonteCarlo::PagodaPathPricer;
        using MonteCarlo::MultiFactorMonteCarloOption;
        using Math::Matrix;

        Pagoda::Pagoda(const Array& portfolio, double fraction, double roof,
                       const Array& dividendYield, const Matrix& covariance,
                       Rate riskFreeRate, const std::vector<Time>& times,
                       unsigned int samples, bool antitheticVariance,
                       long seed) {
            QL_REQUIRE(samples >= 30,
                "Pagoda: less than 30 samples. Are you joking?");
            QL_REQUIRE(covariance.rows() == covariance.columns(),
                "Pagoda: covariance matrix not square");
            QL_REQUIRE(covariance.rows() == portfolio.size(),
                "Pagoda: underlying size does not match that of"
                " covariance matrix");
            QL_REQUIRE(covariance.rows() == dividendYield.size(),
                "Pagoda: dividendYield size does not match"
                " that of covariance matrix");
            QL_REQUIRE(fraction > 0,
                "Pagoda: option fraction must be positive");
            QL_REQUIRE(roof > 0,
                "Pagoda: roof must be positive");
            QL_REQUIRE(times.size() >= 1,
                "Pagoda: you must have at least one time-step");

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
