
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file mceverest.cpp
    \brief %Everest-type option pricer

    \fullpath
    ql/Pricers/%mceverest.cpp
*/

// $Id$

#include <ql/handle.hpp>
#include <ql/MonteCarlo/everestpathpricer.hpp>
#include <ql/MonteCarlo/mctypedefs.hpp>
#include <ql/Pricers/mceverest.hpp>


namespace QuantLib {

    namespace Pricers {

        using Math::Statistics;
        using Math::Matrix;
        using MonteCarlo::MultiPath;
        using MonteCarlo::GaussianMultiPathGenerator;
        using MonteCarlo::PathPricer;
        using MonteCarlo::MonteCarloModel;
        using MonteCarlo::EverestPathPricer;

        McEverest::McEverest(const Array& dividendYield,
            const Matrix& covariance,
            Rate riskFreeRate, Time residualTime,
            bool antitheticVariance, long seed) {

            Size  n = covariance.rows();
            QL_REQUIRE(covariance.columns() == n,
                "McEverest: covariance matrix not square");
            QL_REQUIRE(dividendYield.size() == n,
                "McEverest: dividendYield size does not match"
                " that of covariance matrix");
            QL_REQUIRE(residualTime > 0,
                "McEverest: residualTime must be positive");

            //! Initialize the path generator
            Array mu(riskFreeRate - dividendYield
                                    - 0.5 * covariance.diagonal());

            Handle<GaussianMultiPathGenerator> pathGenerator(
                new GaussianMultiPathGenerator(mu, covariance,
                std::vector<Time>(1, residualTime), seed));

            //! Initialize the pricer on the path pricer
            Handle<PathPricer<MultiPath> > pathPricer(
                new EverestPathPricer(QL_EXP(-riskFreeRate*residualTime),
                antitheticVariance));

             //! Initialize the multi-factor Monte Carlo
            mcModel_ = Handle<MonteCarloModel<Statistics,
                GaussianMultiPathGenerator, PathPricer<MultiPath> > > (
                new MonteCarloModel<Statistics,
                GaussianMultiPathGenerator, PathPricer<MultiPath> > (
                pathGenerator, pathPricer, Statistics()));

        }

    }

}
