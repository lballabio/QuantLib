
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

#include <ql/Pricers/mccliquetoption.hpp>
#include <ql/Instruments/payoffs.hpp>

namespace QuantLib {

    namespace {

        class CliquetOptionPathPricer_old : public PathPricer_old<Path> {
          public:
            CliquetOptionPathPricer_old(
                                 Option::Type type,
                                 double underlying,
                                 double moneyness,
                                 double accruedCoupon,
                                 double lastFixing,
                                 double localCap,
                                 double localFloor,
                                 double globalCap,
                                 double globalFloor,
                                 const std::vector<DiscountFactor>& discounts,
                                 bool redemptionOnly,
                                 bool useAntitheticVariance)
            : PathPricer_old<Path>(1.0, useAntitheticVariance), type_(type),
              underlying_(underlying), moneyness_(moneyness),
              accruedCoupon_(accruedCoupon), lastFixing_(lastFixing),
              localCap_(localCap), localFloor_(localFloor),
              globalCap_(globalCap), globalFloor_(globalFloor), 
              discounts_(discounts), redemptionOnly_(redemptionOnly) {
                QL_REQUIRE(underlying>0.0,
                           "MyCliquet: "
                           "underlying less/equal zero not allowed");

                QL_REQUIRE(moneyness>0.0,
                           "MyCliquet: "
                           "moneyness less/equal zero not allowed");

                if (accruedCoupon == Null<double>())
                    accruedCoupon_ = 0.0;

                if (localCap == Null<double>())
                    localCap_ = QL_MAX_DOUBLE;

                if (localFloor == Null<double>())
                    localFloor_ = 0.0;

                if (globalCap == Null<double>())
                    globalCap_ = QL_MAX_DOUBLE;

                if (globalFloor == Null<double>())
                    globalFloor_ = 0.0;
            }

            double operator()(const Path& path) const {
                Size n = path.size();
                QL_REQUIRE(n>0, "the path cannot be empty");

                QL_REQUIRE(n==discounts_.size(), "discounts/options mismatch");

                double result, result2, lastFixing, underlying, payoff;

                // start the simulation
                lastFixing = lastFixing_;
                underlying = underlying_;
                if (redemptionOnly_)
                    result = accruedCoupon_;
                else
                    result = 0.0;

                Size i;
                // step by step using the discretization of the path
                for (i=0; i<n; i++) {
                    underlying *= QL_EXP(path[i]);
                    // incorporate payoff
                    if (lastFixing != Null<double>()) {
                        payoff =
                            PlainVanillaPayoff(type_,
                                               moneyness_*lastFixing)
                            (underlying) / lastFixing;
                        payoff = QL_MAX(payoff, localFloor_);
                        payoff = QL_MIN(payoff, localCap_);
                        if (redemptionOnly_)
                            result += payoff;
                        else
                            result += payoff * discounts_[i];
                    }
                    // new fixing
                    lastFixing = underlying;
                }
                if (redemptionOnly_) {
                    result = QL_MAX(result, globalFloor_);
                    result = QL_MIN(result, globalCap_);
                }

                if (useAntitheticVariance_) {
                    // start the antothetic simulation
                    lastFixing = lastFixing_;
                    underlying = underlying_;
                    if (redemptionOnly_)
                        result2 = accruedCoupon_;
                    else
                        result2 = 0.0;

                    for (i=0; i<n; i++) {
                        underlying *= QL_EXP(path[i]);
                        // incorporate payoff
                        if (lastFixing != Null<double>()) {
                            payoff =
                                PlainVanillaPayoff(type_,
                                                   moneyness_*lastFixing)
                                (underlying) / lastFixing;
                            payoff = QL_MAX(payoff, localFloor_);
                            payoff = QL_MIN(payoff, localCap_);
                            if (redemptionOnly_)
                                result2 += payoff;
                            else
                                result2 += payoff * discounts_[i];
                        }
                        // new fixing
                        lastFixing = underlying;
                    }
                    if (redemptionOnly_) {
                        result2 = QL_MAX(result2, globalFloor_);
                        result2 = QL_MIN(result2, globalCap_);
                        return discounts_.back()*(result+result2)/2.0;
                    } else {
                        return (result+result2)/2.0;
                    }
                } else {
                    if (redemptionOnly_) {
                        return discounts_.back()*result;
                    } else {
                        return result;
                    }
                }
            }

          private:
            Option::Type type_;
            double underlying_, moneyness_, accruedCoupon_;
            double lastFixing_, localCap_, localFloor_, 
                   globalCap_, globalFloor_;
            std::vector<DiscountFactor> discounts_;
            bool redemptionOnly_;
        };

    }

    McCliquetOption::McCliquetOption(Option::Type type,
                                     double underlying, double moneyness,
                                     const std::vector<Spread>& dividendYield,
                                     const std::vector<Rate>& riskFreeRate,
                                     const std::vector<Time>& times,
                                     const std::vector<double>& volatility,
                                     double accruedCoupon, double lastFixing,
                                     double localCap, double localFloor, 
                                     double globalCap, double globalFloor, 
                                     bool redemptionOnly, 
                                     bool antitheticVariance, long seed) {

        Size dimension = times.size();
        QL_REQUIRE(dividendYield.size()==dimension,
                   "McCliquetOption: dividendYield vector of wrong size");
        QL_REQUIRE(riskFreeRate.size()==dimension,
                   "McCliquetOption: riskFreeRate vector of wrong size");
        QL_REQUIRE(volatility.size()==dimension,
                   "McCliquetOption: volatility vector of wrong size");

        // initialize the path generator
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


        boost::shared_ptr<GaussianPathGenerator_old> pathGenerator(
            new GaussianPathGenerator_old(mu, diffusion,
                                          TimeGrid(times.begin(), times.end()),
                                          seed));

        // initialize the pricer on the single Path
        boost::shared_ptr<PathPricer_old<Path> > cliquetPathPricer(
            new CliquetOptionPathPricer_old(type, underlying, moneyness, 
                                            accruedCoupon, lastFixing,
                                            localCap, localFloor, 
                                            globalCap, globalFloor,
                                            discounts, redemptionOnly, 
                                            antitheticVariance));

        // initialize the one-factor Monte Carlo
        mcModel_ = 
            boost::shared_ptr<MonteCarloModel<SingleAsset_old<
                                                PseudoRandom_old> > >(
                new MonteCarloModel<SingleAsset_old<PseudoRandom_old> >(
                      pathGenerator, cliquetPathPricer, Statistics(), false));
    }

}
