
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
#include <ql/Pricers/mcpagoda.hpp>
#include <ql/TermStructures/flatforward.hpp>

namespace QuantLib {

    namespace {

        class PagodaPathPricer : public PathPricer<MultiPath> {
          public:
            PagodaPathPricer(const std::vector<double>& underlying,
                             double roof, double fraction,
                             const RelinkableHandle<TermStructure>& discountTS)
            : PathPricer<MultiPath>(discountTS),
              underlying_(underlying), roof_(roof), fraction_(fraction) {}

            double operator()(const MultiPath& multiPath) const {
                Size numAssets = multiPath.assetNumber();
                Size numSteps = multiPath.pathSize();
                QL_REQUIRE(underlying_.size() == numAssets,
                           "PagodaPathPricer: the multi-path must contain "
                           + IntegerFormatter::toString(underlying_.size()) +
                           " assets");

                Size i,j;
                double averageGain = 0.0;
                for (i = 0; i < numSteps; i++)
                    for (j = 0; j < numAssets; j++) {
                        averageGain += underlying_[j] *
                            (QL_EXP(multiPath[j][i]) -1.0);
                    }
                return discountTS_->discount(multiPath[0].timeGrid().back())
                    * fraction_ * QL_MAX(0.0, QL_MIN(roof_, averageGain));
            }

          private:
            std::vector<double> underlying_;
            double roof_, fraction_;
        };

    }

    McPagoda::McPagoda(const std::vector<double>& portfolio, double fraction,
                       double roof, const Array& dividendYield, 
                       const Matrix& covariance,
                       Rate riskFreeRate, const std::vector<Time>& times,
                       long seed) {

        QL_REQUIRE(covariance.rows() == covariance.columns(),
                   "McPagoda: covariance matrix not square");
        QL_REQUIRE(covariance.rows() == portfolio.size(),
                   "McPagoda: underlying size does not match that of"
                   " covariance matrix");
        QL_REQUIRE(covariance.rows() == dividendYield.size(),
                   "McPagoda: dividendYield size does not match"
                   " that of covariance matrix");
        QL_REQUIRE(fraction > 0,
                   "McPagoda: option fraction must be positive");
        QL_REQUIRE(roof > 0,
                   "McPagoda: roof must be positive");
        QL_REQUIRE(times.size() >= 1,
                   "McPagoda: you must have at least one time-step");

        // initialize the path generator
        Array mu(riskFreeRate - dividendYield
                 - 0.5 * covariance.diagonal());

        boost::shared_ptr<GaussianMultiPathGenerator> pathGenerator(
            new GaussianMultiPathGenerator(
                mu, covariance,
                TimeGrid(times.begin(), times.end()),
                seed));

        RelinkableHandle<TermStructure> discount(
                  Handle<TermStructure>(
                      new FlatForward(Date::todaysDate(), Date::todaysDate(), 
                                      riskFreeRate)));

        // initialize the pricer on the path pricer
        boost::shared_ptr<PathPricer<MultiPath> > pathPricer(
            new PagodaPathPricer(portfolio, roof, fraction, discount));

         // initialize the multi-factor Monte Carlo
        mcModel_ = boost::shared_ptr<MonteCarloModel<MultiAsset_old<
                                          PseudoRandomSequence_old> > > (
            new MonteCarloModel<MultiAsset_old<
                                PseudoRandomSequence_old> > (
                pathGenerator, pathPricer, Statistics(), false));

    }

}
