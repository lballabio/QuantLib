
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

#include <ql/Pricers/mcpagoda.hpp>

namespace QuantLib {

    namespace {

        class PagodaPathPricer : public PathPricer<MultiPath> {
          public:
            PagodaPathPricer(const std::vector<Real>& underlying,
                             Real roof, Real fraction,
                             DiscountFactor discount)
            : underlying_(underlying), roof_(roof), fraction_(fraction),
              discount_(discount) {}

            Real operator()(const MultiPath& multiPath) const {
                Size numAssets = multiPath.assetNumber();
                Size numSteps = multiPath.pathSize();
                QL_REQUIRE(underlying_.size() == numAssets,
                           "the multi-path must contain "
                           << underlying_.size() << " assets");

                Size i,j;
                Real averageGain = 0.0;
                for (i = 0; i < numSteps; i++)
                    for (j = 0; j < numAssets; j++) {
                        averageGain += underlying_[j] *
                            (std::exp(multiPath[j][i]) -1.0);
                    }
                return discount_ * fraction_
                    * std::max<Real>(0.0, std::min(roof_, averageGain));
            }

          private:
            std::vector<Real> underlying_;
            Real roof_, fraction_;
            DiscountFactor discount_;
        };

    }

    McPagoda::McPagoda(
              const std::vector<Real>& underlying,
              Real fraction,
              Real roof,
              const std::vector<Handle<YieldTermStructure> >& dividendYield,
              const Handle<YieldTermStructure>& riskFreeRate,
              const std::vector<Handle<BlackVolTermStructure> >& volatilities,
              const Matrix& correlation,
              const std::vector<Time>& times,
              BigNatural seed) {

        QL_REQUIRE(correlation.rows() == correlation.columns(),
                   "correlation matrix not square");
        QL_REQUIRE(correlation.rows() == underlying.size(),
                   "underlying size does not match that of"
                   " correlation matrix");
        QL_REQUIRE(correlation.rows() == dividendYield.size(),
                   "dividendYield size does not match"
                   " that of correlation matrix");
        QL_REQUIRE(fraction > 0,
                   "option fraction must be positive");
        QL_REQUIRE(roof > 0,
                   "roof must be positive");
        QL_REQUIRE(times.size() >= 1,
                   "you must have at least one time-step");

        // initialize the path generator
        Size n = underlying.size();
        std::vector<boost::shared_ptr<StochasticProcess> > processes(n);
        for (Size i=0; i<n; i++) {
            Handle<Quote> u(
                    boost::shared_ptr<Quote>(new SimpleQuote(underlying[i])));
            processes[i] = boost::shared_ptr<StochasticProcess>(
                                    new BlackScholesProcess(u,
                                                            dividendYield[i],
                                                            riskFreeRate,
                                                            volatilities[i]));
        }

        TimeGrid grid(times.begin(), times.end());
        PseudoRandom::rsg_type rsg =
            PseudoRandom::make_sequence_generator(n*(grid.size()-1),seed);

        bool brownianBridge = false;

        typedef MultiAsset<PseudoRandom>::path_generator_type generator;
        boost::shared_ptr<generator> pathGenerator(new
            generator(processes, correlation, grid, rsg, brownianBridge));

        // initialize the path pricer
        DiscountFactor discount = riskFreeRate->discount(times.back());
        boost::shared_ptr<PathPricer<MultiPath> > pathPricer(
                  new PagodaPathPricer(underlying, roof, fraction, discount));

         // initialize the multi-factor Monte Carlo
        mcModel_ = boost::shared_ptr<MonteCarloModel<MultiAsset<
                                                     PseudoRandom> > > (
            new MonteCarloModel<MultiAsset<PseudoRandom> > (
                             pathGenerator, pathPricer, Statistics(), false));

    }

}
