
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

#include <ql/Pricers/mcdiscretearithmeticapo.hpp>
#include <ql/Pricers/discretegeometricapo.hpp>
#include <ql/PricingEngines/Asian/mc_discr_geom_av_price.hpp>
#include <ql/PricingEngines/Asian/mc_discr_arith_av_price.hpp>

namespace QuantLib {


    #ifndef QL_DISABLE_DEPRECATED
    McDiscreteArithmeticAPO::McDiscreteArithmeticAPO(
                              Option::Type type,
                              Real underlying,
                              Real strike,
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
        boost::shared_ptr<StochasticProcess> diffusion(
                                     new BlackScholesProcess(u,
                                                             dividendYield,
                                                             riskFreeRate,
                                                             volatility));
        TimeGrid grid(times.begin(), times.end());
        PseudoRandom::rsg_type rsg =
            PseudoRandom::make_sequence_generator(grid.size()-1,seed);

        typedef SingleAsset<PseudoRandom>::path_generator_type generator;
        boost::shared_ptr<generator> pathGenerator(
                                  new generator(diffusion, grid, rsg, false));

        // initialize the path pricer
        DiscountFactor discount = riskFreeRate->discount(times.back());
        boost::shared_ptr<PathPricer<Path> > spPricer(
                           new ArithmeticAPOPathPricer(type, underlying,
                                                       strike, discount));

        if (controlVariate) {
            boost::shared_ptr<PathPricer<Path> > controlVariateSpPricer(
                            new GeometricAPOPathPricer(type, underlying,
                                                       strike, discount));

            // Not sure whether this work when curves are not flat...
            Time exercise = times.back();
            Rate r = riskFreeRate->zeroYield(exercise);
            Rate q = dividendYield->zeroYield(exercise);
            Volatility sigma = volatility->blackVol(exercise,underlying);

            Real controlVariatePrice = DiscreteGeometricAPO(type,
                underlying, strike, q, r, times, sigma).value();

            /*
            boost::shared_ptr<PlainVanillaPayoff> payoff(new
                PlainVanillaPayoff(type, strike));
            boost::shared_ptr<PricingEngine> engine(new
                AnalyticDiscreteAveragingAsianEngine);
            DiscreteAveragingAsianOption option(Average::Geometric,
                0.0, 0, fixingDates,
                diffusion, payoff, exercise, engine);
            Real controlVariatePrice = option.NPV();
            */

            // initialize the Monte Carlo model
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset<
                                              PseudoRandom> > >(
                new MonteCarloModel<SingleAsset<PseudoRandom> >(
                    pathGenerator, spPricer, Statistics(), false,
                    controlVariateSpPricer, controlVariatePrice));
        } else {
            // initialize the Monte Carlo model
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset<
                                              PseudoRandom> > > (
                new MonteCarloModel<SingleAsset<PseudoRandom> >(
                    pathGenerator, spPricer, Statistics(), false));
        }

    }

    #endif // QL_DISABLE_DEPRECATED
}
