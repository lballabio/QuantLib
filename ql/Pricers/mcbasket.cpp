
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/handle.hpp>
#include <ql/Pricers/mcbasket.hpp>
#include <ql/Instruments/payoffs.hpp>
#include <ql/TermStructures/flatforward.hpp>

namespace QuantLib {

    namespace {

        class BasketPathPricer : public PathPricer<MultiPath> {
          public:
            BasketPathPricer(Option::Type type,
                             const std::vector<double>& underlying,
                             double strike,
                             const RelinkableHandle<TermStructure>& discountTS)
            : PathPricer<MultiPath>(discountTS),
              underlying_(underlying), payoff_(type, strike) {
                for (Size j=0; j<underlying_.size(); j++) {
                    QL_REQUIRE(underlying_[j]>0.0,
                               "BasketPathPricer: "
                               "underlying less/equal zero not allowed");
                    QL_REQUIRE(strike>=0.0,
                               "BasketPathPricer: "
                               "strike less than zero not allowed");
                }
            }

            double operator()(const MultiPath& multiPath) const {

                Size numSteps = multiPath.pathSize();
                Size numAssets = multiPath.assetNumber();
                QL_REQUIRE(underlying_.size() == numAssets,
                           "BasketPathPricer: the multi-path must contain "
                           + IntegerFormatter::toString(underlying_.size()) 
                           + " assets");

                QL_REQUIRE(numSteps>0,
                           "BasketPathPricer: the path cannot be empty");

                // start the simulation
                std::vector<double> log_variation(numAssets, 0.0);
                Size i,j;
                double basketPrice = 0.0;
                for (j = 0; j < numAssets; j++) {
                    log_variation[j] = 0.0;
                    for (i = 0; i < numSteps; i++)
                        log_variation[j] += multiPath[j][i];
                    basketPrice += underlying_[j]*
                        QL_EXP(log_variation[j]);
                }
                return discountTS_->discount(multiPath[0].timeGrid().back())
                    * payoff_(basketPrice);
            }

          private:
            std::vector<double> underlying_;
            PlainVanillaPayoff payoff_;
        };

    }


    McBasket::McBasket(Option::Type type, 
                       const std::vector<double>& underlying,
                       double strike, const Array& dividendYield,
                       const Matrix& covariance,
                       Rate riskFreeRate,  double residualTime,
                       long seed) {

        QL_REQUIRE(covariance.rows() == covariance.columns(),
                   "McBasket: covariance matrix not square");
        QL_REQUIRE(covariance.rows() == underlying.size(),
                   "McBasket: underlying size does not match that of"
                   " covariance matrix");
        QL_REQUIRE(covariance.rows() == dividendYield.size(),
                   "McBasket: dividendYield size does not match"
                   " that of covariance matrix");
        QL_REQUIRE(residualTime > 0,
                   "McBasket: residual time must be positive");

        // initialize the path generator
        Array mu(riskFreeRate - dividendYield
                 - 0.5 * covariance.diagonal());

        boost::shared_ptr<GaussianMultiPathGenerator> pathGenerator(
            new GaussianMultiPathGenerator(mu, covariance,
                                           TimeGrid(residualTime, 1), seed));

        RelinkableHandle<TermStructure> discount(
                  Handle<TermStructure>(
                      new FlatForward(Date::todaysDate(), Date::todaysDate(), 
                                      riskFreeRate)));

        // initialize the path pricer
        boost::shared_ptr<PathPricer<MultiPath> > pathPricer(
            new BasketPathPricer(type, underlying, strike, discount));

        // initialize the multi-factor Monte Carlo
        mcModel_ = boost::shared_ptr<MonteCarloModel
                             <MultiAsset_old<PseudoRandomSequence_old> > >(
            new MonteCarloModel<MultiAsset_old<PseudoRandomSequence_old> >(
                             pathGenerator, pathPricer, Statistics(), false));
    }

}
