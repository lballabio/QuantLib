
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

#include <ql/Pricers/mcdiscretearithmeticaso.hpp>
#include <ql/Pricers/discretegeometricaso.hpp>

namespace QuantLib {

    namespace {

        class ArithmeticASOPathPricer_old : public PathPricer_old<Path> {
          public:
            ArithmeticASOPathPricer_old(Option::Type type,
                                        double underlying,
                                        DiscountFactor discount,
                                        bool useAntitheticVariance)
            : PathPricer_old<Path>(discount, useAntitheticVariance), 
              type_(type), underlying_(underlying) {
                QL_REQUIRE(underlying>0.0,
                           "ArithmeticASOPathPricer_old: "
                           "underlying less/equal zero not allowed");
            }

            double operator()(const Path& path) const  {

                Size n = path.size();
                QL_REQUIRE(n > 0,
                           "ArithmeticASOPathPricer_old: "
                           "the path cannot be empty");

                double price1 = underlying_;
                double averageStrike1 = 0.0;
                Size fixings = n;
                if (path.timeGrid().mandatoryTimes()[0]==0.0) {
                    averageStrike1 = price1;
                    fixings = n+1;
                }
                Size i;
                for (i=0; i<n; i++) {
                    price1 *= QL_EXP(path.drift()[i]+path.diffusion()[i]);
                    averageStrike1 += price1;
                }
                averageStrike1 = averageStrike1/fixings;

                if (useAntitheticVariance_) {
                    double price2 = underlying_;
                    double averageStrike2 = 0.0;

                    if (path.timeGrid().mandatoryTimes()[0]==0.0)
                        averageStrike2 = price2;
                    for (i=0; i<n; i++) {
                        price2 *= QL_EXP(path.drift()[i]-path.diffusion()[i]);
                        averageStrike2 += price2;
                    }
                    averageStrike2 = averageStrike2/fixings;
                    return discount_/2.0 * 
                        (PlainVanillaPayoff(type_, averageStrike1)(price1)
                         + PlainVanillaPayoff(type_, averageStrike2)(price2));
                } else {
                    return discount_ * 
                        PlainVanillaPayoff(type_, averageStrike1)(price1);
                }
            }

          private:
            Option::Type type_;
            double underlying_;
        };

        class GeometricASOPathPricer_old : public PathPricer_old<Path> {
          public:
            GeometricASOPathPricer_old(Option::Type type,
                                       double underlying,
                                       DiscountFactor discount,
                                       bool useAntitheticVariance)
            : PathPricer_old<Path>(discount, useAntitheticVariance), 
              type_(type), underlying_(underlying) {
                QL_REQUIRE(underlying>0.0,
                           "GeometricASOPathPricer_old: "
                           "underlying less/equal zero not allowed");
            }

            double operator()(const Path& path) const {

                Size n = path.size();
                QL_REQUIRE(n>0,
                           "GeometricASOPathPricer_old: "
                           "the path cannot be empty");

                double logDrift = 0.0, logDiffusion = 0.0;
                double geoLogDrift = 0.0, geoLogDiffusion = 0.0;
                Size i;
                for (i=0; i<n; i++) {
                    logDrift += path.drift()[i];
                    logDiffusion += path.diffusion()[i];
                    geoLogDrift += (n-i)*path.drift()[i];
                    geoLogDiffusion += (n-i)*path.diffusion()[i];
                }
                Size fixings = n;
                if (path.timeGrid().mandatoryTimes()[0]==0.0) {
                    fixings = n+1;
                }
                double averageStrike1 = underlying_*
                    QL_EXP((geoLogDrift+geoLogDiffusion)/fixings);

                if (useAntitheticVariance_) {
                    double averageStrike2 = underlying_*
                        QL_EXP((geoLogDrift-geoLogDiffusion)/fixings);
                    return discount_* 0.5 *
                        (PlainVanillaPayoff(type_, averageStrike1)
                         (underlying_ * QL_EXP(logDrift+logDiffusion))
                         + PlainVanillaPayoff(type_, averageStrike2)
                         (underlying_ * QL_EXP(logDrift-logDiffusion)));
                } else {
                    return discount_*PlainVanillaPayoff(type_, averageStrike1)
                        (underlying_ * QL_EXP(logDrift+logDiffusion));
                }
            }

          private:
            Option::Type type_;
            double underlying_;
        };

    }

    McDiscreteArithmeticASO::McDiscreteArithmeticASO(Option::Type type,
      double underlying,
      Spread dividendYield, Rate riskFreeRate,
      const std::vector<Time>& times, double volatility,
      bool antitheticVariance, bool controlVariate, long seed) {

        QL_REQUIRE(times.size() >= 2,
                   "McDiscreteArithmeticASO: "
                   "you must have at least 2 time-steps");

        // initialize the path generator
        double mu = riskFreeRate - dividendYield
                                 - 0.5 * volatility * volatility;

        boost::shared_ptr<GaussianPathGenerator_old> pathGenerator(
            new GaussianPathGenerator_old(mu, volatility*volatility,
                TimeGrid(times.begin(), times.end()),
                seed));

        // initialize the Path Pricer
        boost::shared_ptr<PathPricer_old<Path> > spPricer(
            new ArithmeticASOPathPricer_old(type, underlying,
                QL_EXP(-riskFreeRate*times.back()), antitheticVariance));

        if (controlVariate) {
            boost::shared_ptr<PathPricer_old<Path> > controlVariateSpPricer(
                new GeometricASOPathPricer_old(type, underlying,
                QL_EXP(-riskFreeRate*times.back()),
                antitheticVariance));

            double controlVariatePrice = DiscreteGeometricASO(type,
                underlying, dividendYield, riskFreeRate,
                times, volatility).value();

            // initialize the one-dimensional Monte Carlo
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset_old<
                                              PseudoRandom_old> > > (
                new MonteCarloModel<SingleAsset_old<PseudoRandom_old> >(
                    pathGenerator, spPricer, Statistics(), false,
                    controlVariateSpPricer, controlVariatePrice));
        } else {
            // initialize the one-dimensional Monte Carlo
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset_old<
                                              PseudoRandom_old> > > (
                new MonteCarloModel<SingleAsset_old<PseudoRandom_old> >(
                    pathGenerator, spPricer, Statistics(), false));
        }

    }

}
