
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
                           + SizeFormatter::toString(underlying_.size()) +
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

    McPagoda::McPagoda(
                 const std::vector<double>& underlying,
                 double fraction,
                 double roof,
                 const std::vector<RelinkableHandle<TermStructure> >& 
                                                             dividendYield,
                 const RelinkableHandle<TermStructure>& riskFreeRate,
                 const std::vector<RelinkableHandle<BlackVolTermStructure> >& 
                                                             volatilities,
                 const Matrix& correlation,
                 const std::vector<Time>& times,
                 long seed) {

        QL_REQUIRE(correlation.rows() == correlation.columns(),
                   "McPagoda: correlation matrix not square");
        QL_REQUIRE(correlation.rows() == underlying.size(),
                   "McPagoda: underlying size does not match that of"
                   " correlation matrix");
        QL_REQUIRE(correlation.rows() == dividendYield.size(),
                   "McPagoda: dividendYield size does not match"
                   " that of correlation matrix");
        QL_REQUIRE(fraction > 0,
                   "McPagoda: option fraction must be positive");
        QL_REQUIRE(roof > 0,
                   "McPagoda: roof must be positive");
        QL_REQUIRE(times.size() >= 1,
                   "McPagoda: you must have at least one time-step");

        // initialize the path generator
        Size n = underlying.size();
        std::vector<boost::shared_ptr<DiffusionProcess> > processes(n);
        for (Size i=0; i<n; i++)
            processes[i] = Handle<DiffusionProcess>(
                    new BlackScholesProcess(riskFreeRate, dividendYield[i],
                                            volatilities[i], underlying[i]));

        TimeGrid grid(times.begin(), times.end());
        PseudoRandom::rsg_type rsg = 
            PseudoRandom::make_sequence_generator(n*(grid.size()-1),seed);

        typedef MultiAsset<PseudoRandom>::path_generator_type generator;
        boost::shared_ptr<generator> pathGenerator(
                                  new generator(processes, correlation, grid, 
                                                rsg, false));

        // initialize the pricer on the path pricer
        boost::shared_ptr<PathPricer<MultiPath> > pathPricer(
            new PagodaPathPricer(underlying, roof, fraction, riskFreeRate));

         // initialize the multi-factor Monte Carlo
        mcModel_ = boost::shared_ptr<MonteCarloModel<MultiAsset<
                                                     PseudoRandom> > > (
            new MonteCarloModel<MultiAsset<PseudoRandom> > (
                             pathGenerator, pathPricer, Statistics(), false));

    }

}
