/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Pricers/mcdiscretearithmeticaso.hpp>
#include <ql/Pricers/discretegeometricaso.hpp>
#include <ql/Processes/blackscholesprocess.hpp>

namespace QuantLib {

    namespace {

        class ArithmeticASOPathPricer : public PathPricer<Path> {
          public:
            ArithmeticASOPathPricer(Option::Type type,
                                    DiscountFactor discount)
            : type_(type), discount_(discount) {}

            Real operator()(const Path& path) const  {
                Size n = path.length();
                QL_REQUIRE(n > 1, "the path cannot be empty");

                Real averageStrike;
                if (path.timeGrid().mandatoryTimes()[0]==0.0) {
                    // include initial fixing
                    averageStrike =
                        std::accumulate(path.begin(),path.end(),0.0)/n;
                } else {
                    averageStrike =
                        std::accumulate(path.begin()+1,path.end(),0.0)/(n-1);
                }

                return discount_
                    * PlainVanillaPayoff(type_, averageStrike)(path.back());
            }

          private:
            Option::Type type_;
            DiscountFactor discount_;
        };

        class GeometricASOPathPricer : public PathPricer<Path> {
          public:
            GeometricASOPathPricer(Option::Type type,
                                   DiscountFactor discount)
            : type_(type), discount_(discount) {}

            Real operator()(const Path& path) const {

                Size n = path.length();
                QL_REQUIRE(n>1, "the path cannot be empty");

                Size fixings = n-1;
                if (path.timeGrid().mandatoryTimes()[0]==0.0)
                    fixings = n;

                Real averageStrike = 1.0;
                Real maxValue = QL_MAX_REAL;
                Real product = 1.0;
                if (path.timeGrid().mandatoryTimes()[0]==0.0)
                    product = path.front();
                for (Size i=1; i<n; i++) {
                    Real value = path[i];
                    if (product < maxValue/value) {
                        product *= value;
                    } else {
                        averageStrike *= std::pow(product, 1.0/fixings);
                        product = value;
                    }
                }
                averageStrike *= std::pow(product, 1.0/fixings);
                return discount_
                    * PlainVanillaPayoff(type_, averageStrike)(path.back());
            }
          private:
            Option::Type type_;
            DiscountFactor discount_;
        };

    }

    McDiscreteArithmeticASO::McDiscreteArithmeticASO(
                              Option::Type type,
                              Real underlying,
                              const Handle<YieldTermStructure>& dividendYield,
                              const Handle<YieldTermStructure>& riskFreeRate,
                              const Handle<BlackVolTermStructure>& volatility,
                              const std::vector<Time>& times,
                              bool controlVariate,
                              BigNatural seed) {

        QL_REQUIRE(times.size() >= 2,
                   "you must have at least 2 time-steps");

        // initialize the path generator
        Handle<Quote> u(boost::shared_ptr<Quote>(new SimpleQuote(underlying)));
        boost::shared_ptr<StochasticProcess1D> diffusion(
                          new GeneralizedBlackScholesProcess(u,
                                                             dividendYield,
                                                             riskFreeRate,
                                                             volatility));
        TimeGrid grid(times.begin(), times.end());
        PseudoRandom::rsg_type rsg =
            PseudoRandom::make_sequence_generator(grid.size()-1,seed);

        bool brownianBridge = false;

        typedef SingleVariate<PseudoRandom>::path_generator_type generator;
        boost::shared_ptr<generator> pathGenerator(new
            generator(diffusion, grid, rsg, brownianBridge));

        // initialize the path pricer
        DiscountFactor discount = riskFreeRate->discount(times.back());
        boost::shared_ptr<PathPricer<Path> > spPricer(
                                 new ArithmeticASOPathPricer(type, discount));

        if (controlVariate) {
            boost::shared_ptr<PathPricer<Path> > controlVariateSpPricer(
                                  new GeometricASOPathPricer(type, discount));

            // Not sure whether this work when curves are not flat...
            Time exercise = times.back();
            Rate r = riskFreeRate->zeroRate(exercise,Continuous,NoFrequency);
            Rate q = dividendYield->zeroRate(exercise,Continuous,NoFrequency);
            Volatility sigma = volatility->blackVol(exercise,underlying);

            Real controlVariatePrice = DiscreteGeometricASO(type,
                underlying, q, r, times, sigma).value();

            // initialize the one-dimensional Monte Carlo
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleVariate<
                                              PseudoRandom> > > (
                new MonteCarloModel<SingleVariate<PseudoRandom> >(
                    pathGenerator, spPricer, Statistics(), false,
                    controlVariateSpPricer, controlVariatePrice));
        } else {
            // initialize the one-dimensional Monte Carlo
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleVariate<
                                              PseudoRandom> > > (
                new MonteCarloModel<SingleVariate<PseudoRandom> >(
                    pathGenerator, spPricer, Statistics(), false));
        }

    }

}
