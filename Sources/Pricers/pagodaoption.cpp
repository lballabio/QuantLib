
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

    $Id$
*/

// $Source$
// $Log$
// Revision 1.13  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.12  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.11  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.10  2001/07/13 15:25:13  marmar
// MonteCarlo interface changed
//
// Revision 1.9  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.8  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/handle.hpp"
#include "ql/Pricers/pagodaoption.hpp"
#include "ql/MonteCarlo/pagodapathpricer.hpp"

namespace QuantLib {

    namespace Pricers {

        using MonteCarlo::MultiPathPricer;
        using MonteCarlo::StandardMultiPathGenerator;
        using MonteCarlo::PagodaPathPricer;
        using MonteCarlo::MultiFactorMonteCarloOption;

        PagodaOption::PagodaOption(
            const Array &portfolio,  double fraction,
            double roof, double residualTime,
            const Math::Matrix &covariance,
            const Array &dividendYield, Rate riskFreeRate,
            int timesteps, long samples, long seed)
        : MultiFactorPricer(samples, seed){
            QL_REQUIRE(covariance.rows() == covariance.columns(),
                "PagodaOption: covariance matrix not square");
            QL_REQUIRE(covariance.rows() == portfolio.size(),
                "PagodaOption: underlying size does not match that of"
                " covariance matrix");
            QL_REQUIRE(covariance.rows() == dividendYield.size(),
                "PagodaOption: dividendYield size does not match"
                " that of covariance matrix");
            QL_REQUIRE(residualTime > 0,
                "PagodaOption: residual time must be positive");
            QL_REQUIRE(fraction > 0,
                "PagodaOption: option fraction must be positive");
            QL_REQUIRE(roof > 0,
                "PagodaOption: roof must be positive");
            QL_REQUIRE(timesteps > 0,
                "PagodaOption: time steps must be positive");

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
            Handle<MultiPathPricer> pathPricer(
                new PagodaPathPricer(portfolio, roof,
                        fraction * QL_EXP(-riskFreeRate*residualTime)));

             //! Initialize the multi-factor Monte Carlo
            montecarloPricer_ = MultiFactorMonteCarloOption(
                                        pathGenerator, pathPricer,
                                        Math::Statistics());
        }

    }

}
