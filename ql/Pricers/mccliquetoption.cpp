
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file mccliquetoption.cpp
    \brief Cliquet option priced with Monte Carlo simulation

    \fullpath
    ql/Pricers/%mccliquetoption.cpp
*/

// $Id$

#include <ql/Pricers/mccliquetoption.hpp>
#include <ql/MonteCarlo/cliquetoptionpathpricer.hpp>

namespace QuantLib {

    namespace Pricers {

        using namespace Math;
        using namespace MonteCarlo;

        McCliquetOption::McCliquetOption(Option::Type type,
          double underlying, double moneyness,
          const std::vector<Spread>& dividendYield,
          const std::vector<Rate>& riskFreeRate,
          const std::vector<Time>& times,
          const std::vector<double>& volatility,
          double accruedCoupon, double lastFixing,
          double localCap, double localFloor, double globalCap,
          double globalFloor, bool redemptionOnly, 
          bool antitheticVariance, long seed) {

            Size dimension = times.size();
            QL_REQUIRE(dividendYield.size()==dimension,
                "McCliquetOption: dividendYield vector of wrong size");
            QL_REQUIRE(riskFreeRate.size()==dimension,
                "McCliquetOption: riskFreeRate vector of wrong size");
            QL_REQUIRE(volatility.size()==dimension,
                "McCliquetOption: volatility vector of wrong size");

            //! Initialize the path generator
            std::vector<double> mu(dimension);
            std::vector<double> diffusion(dimension);
            std::vector<double> discounts(dimension);
            for (Size i = 0; i<dimension; i++) {
                mu[i]= riskFreeRate[i] - dividendYield[i] -
                    0.5 * volatility[i] * volatility[i];
                diffusion[i]= volatility[i] * volatility[i];
                if (i==0)
                    discounts[i] = QL_EXP(-riskFreeRate[i]*times[i]);
                else
                    discounts[i] = discounts[i-1]*
                        QL_EXP(-riskFreeRate[i]*(times[i]-times[i-1]));
            }


            Handle<GaussianPathGenerator_old> pathGenerator(
                new GaussianPathGenerator_old(mu, diffusion,
                    TimeGrid(times.begin(), times.end()),
                    seed));

            //! Initialize the pricer on the single Path
            Handle<PathPricer_old<Path> > cliquetPathPricer(
                new CliquetOptionPathPricer_old(type,
                underlying, moneyness, accruedCoupon, lastFixing,
                localCap, localFloor, globalCap, globalFloor,
                discounts, redemptionOnly, antitheticVariance));

            //! Initialize the one-factor Monte Carlo
            mcModel_ = Handle<MonteCarloModel<Statistic,
                GaussianPathGenerator_old, PathPricer_old<Path> > > (
                new MonteCarloModel<Statistic,
                GaussianPathGenerator_old, PathPricer_old<Path> > (
                pathGenerator, cliquetPathPricer,
                Statistic(), false));

        }

    }

}
