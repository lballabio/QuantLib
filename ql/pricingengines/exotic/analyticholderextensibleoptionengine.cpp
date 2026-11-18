/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/exercise.hpp>
#include <ql/math/distributions/bivariatenormaldistribution.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/functional.hpp>
#include <ql/pricingengines/exotic/analyticholderextensibleoptionengine.hpp>
#include <cmath>
#include <utility>

using std::pow;
using std::log;
using std::exp;
using std::sqrt;

namespace QuantLib {

    namespace {

        // maximum number of Newton-Raphson steps to find a critical price
        const Size maxIterations = 100;

        Real nextCriticalPrice(Real spot, Real value, Real derivative) {
            QL_REQUIRE(std::isfinite(value), "critical price: non-finite residual");
            QL_REQUIRE(std::isfinite(derivative) && derivative != 0.0,
                       "critical price: zero or non-finite derivative");
            Real next = spot - value / derivative;
            QL_REQUIRE(std::isfinite(next) && next > 0.0,
                       "critical price: non-positive or non-finite iterate");
            return next;
        }

    }

    AnalyticHolderExtensibleOptionEngine::AnalyticHolderExtensibleOptionEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void AnalyticHolderExtensibleOptionEngine::calculate() const {
        //Spot
        Real S = process_->x0();
        Real r = riskFreeRate();
        Real b = r - dividendYield();
        Real X1 = strike();
        Real X2 = arguments_.secondStrike;
        Time T2 = secondExpiryTime();
        Time t1 = firstExpiryTime();
        Real A = arguments_.premium;


        Real z1 = this->z1();

        Real z2 = this->z2();

        Real rho = sqrt(t1 / T2);


        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);

        //QuantLib requires sigma * sqrt(T) rather than just sigma/volatility
        Real vol = volatility();

        //calculate dividend discount factor assuming continuous compounding (e^-rt)
        DiscountFactor growth = dividendDiscount(t1);
        //calculate payoff discount factor assuming continuous compounding
        DiscountFactor discount = riskFreeDiscount(t1);
        Real result = 0;
        constexpr double minusInf = -std::numeric_limits<double>::infinity();

        // Check whether the standard extension region is empty.
        if (bsCalculator(X1, payoff->optionType()).value() <= A) {
            QL_REQUIRE(payoff->optionType() != Option::Call ||
                           dividendDiscount(T2 - t1) <= 1.0,
                       "holder-extensible call extension only at high spots is not supported");
            results_.value = BlackScholesCalculator(payoff, S, growth, vol*sqrt(t1), discount).value();
            return;
        }

        Real y1 = this->y1(payoff->optionType()),
             y2 = this->y2(payoff->optionType());
        if (payoff->optionType() == Option::Call) {
            //instantiate payoff function for a call
            ext::shared_ptr<PlainVanillaPayoff> vanillaCallPayoff =
                ext::make_shared<PlainVanillaPayoff>(Option::Call, X1);
            Real BSM = BlackScholesCalculator(vanillaCallPayoff, S, growth, vol*sqrt(t1), discount).value();
            result = BSM
                + S*exp((b - r)*T2)*M2(y1, y2, minusInf, z1, rho)
                - X2*exp(-r*T2)*M2(y1 - vol*sqrt(t1), y2 - vol*sqrt(t1), minusInf, z1 - vol*sqrt(T2), rho)
                - S*exp((b - r)*t1)*N2(y1, z2) + X1*exp(-r*t1)*N2(y1 - vol*sqrt(t1), z2 - vol*sqrt(t1))
                - A*exp(-r*t1)*N2(y1 - vol*sqrt(t1), y2 - vol*sqrt(t1));
        } else {
            //instantiate payoff function for a call
            ext::shared_ptr<PlainVanillaPayoff> vanillaPutPayoff =
                ext::make_shared<PlainVanillaPayoff>(Option::Put, X1);
            result = BlackScholesCalculator(vanillaPutPayoff, S, growth, vol*sqrt(t1), discount).value()
                - S*exp((b - r)*T2)*M2(y1, y2, minusInf, -z1, -rho)
                + X2*exp(-r*T2)*M2(y1 - vol*sqrt(t1), y2 - vol*sqrt(t1), minusInf, -z1 + vol*sqrt(T2), -rho)
                + S*exp((b - r)*t1)*N2(z2, y2) - X1*exp(-r*t1)*N2(z2 - vol*sqrt(t1), y2 - vol*sqrt(t1))
                - A*exp(-r*t1)*N2(y1 - vol*sqrt(t1), y2 - vol*sqrt(t1));
        }
        this->results_.value = result;
    }

    Real AnalyticHolderExtensibleOptionEngine::I1Call() const {
        Real Sv = strike();
        Real A = arguments_.premium;

        if(A==0)
        {
            return 0;
        }
        else
        {
            BlackScholesCalculator bs = bsCalculator(Sv, Option::Call);
            Real ci = bs.value();
            Real dc = bs.delta();

            Real yi = ci - A;
            //da/ds = 0
            Real di = dc - 0;
            Real epsilon = 0.001;

            //Newton-Raphson process
            Size iterations = 0;
            while (std::fabs(yi) > epsilon || !std::isfinite(yi)) {
                QL_REQUIRE(++iterations <= maxIterations,
                           "lower critical price: max iterations exceeded");
                Sv = nextCriticalPrice(Sv, yi, di);

                bs = bsCalculator(Sv, Option::Call);
                ci = bs.value();
                dc = bs.delta();

                yi = ci - A;
                di = dc - 0;
            }
            return Sv;
        }
    }

    Real AnalyticHolderExtensibleOptionEngine::I2Call() const {
        Real Sv = strike();
        Real X1 = strike();
        Real X2 = arguments_.secondStrike;
        Real A = arguments_.premium;
        Time T2 = secondExpiryTime();
        Time t1 = firstExpiryTime();
        Real r=riskFreeRate();

        Real val=X1-X2*std::exp(-r*(T2-t1));
        DiscountFactor growth = dividendDiscount(T2 - t1);
        // with dividends, exercise beats extension at a high enough spot
        if (A < val && growth >= 1.0) {
            return std::numeric_limits<Real>::infinity();
        } else {
            if (growth > 1.0) {
                /* With a negative dividend yield, the function solved for below, c(S) - A - S + X1,
                   is convex but its slope tends to growth - 1 > 0, so it can have no zero. Its
                   minimum is where the call delta, growth*N(d1), equals 1; since c(S) is then
                   S - X2*D_r*N(d2), the minimum is X1 - A - X2*D_r*N(d2). If it is not negative,
                   or if X1 (where the function is positive) lies to the right of the minimum and
                   thus of any zero, exercise never beats extension. */
                Real stdDev = volatility() * std::sqrt(T2 - t1);
                DiscountFactor discount = riskFreeDiscount(T2 - t1);
                Real d1 = InverseCumulativeNormal()(1.0 / growth);
                Real Smin = X2 * std::exp(d1 * stdDev - 0.5 * stdDev * stdDev) * discount / growth;
                Real minimum = X1 - A - X2 * discount * CumulativeNormalDistribution()(d1 - stdDev);
                if (minimum >= 0.0 || X1 >= Smin)
                    return std::numeric_limits<Real>::infinity();

                // The second extension region needs an additional critical price.
                QL_FAIL("holder-extensible call with two roots is not supported");
            }

            BlackScholesCalculator bs = bsCalculator(Sv, Option::Call);
            Real ci = bs.value();
            Real dc = bs.delta();

            Real yi = ci - A - Sv + X1;
            //da/ds = 1
            Real di = dc - 1;
            Real epsilon = 0.001;

            //Newton-Raphson process
            Size iterations = 0;
            while (std::fabs(yi) > epsilon || !std::isfinite(yi)) {
                QL_REQUIRE(++iterations <= maxIterations,
                           "upper critical price: max iterations exceeded");
                Sv = nextCriticalPrice(Sv, yi, di);

                bs = bsCalculator(Sv, Option::Call);
                ci = bs.value();
                dc = bs.delta();

                yi = ci - A - Sv + X1;
                di = dc - 1;
            }
            return Sv;
        }
    }

    Real AnalyticHolderExtensibleOptionEngine::I1Put() const {
        Real Sv = strike();
        //Srtike
        Real X1 = strike();
        //Premium
        Real A = arguments_.premium;
        Real X2 = arguments_.secondStrike;
        Time T2 = secondExpiryTime();
        Time t1 = firstExpiryTime();
        Real r = riskFreeRate();

        if (A <= X2*std::exp(-r*(T2-t1)) - X1)
            return 0.0;

        BlackScholesCalculator bs = bsCalculator(Sv, Option::Put);
        Real pi = bs.value();
        Real dc = bs.delta();

        Real yi = pi - A + Sv - X1;
        //da/ds = 1
        Real di = dc + 1;
        Real epsilon = 0.001;

        //Newton-Raphson prosess
        Size iterations = 0;
        while (std::fabs(yi) > epsilon || !std::isfinite(yi)) {
            QL_REQUIRE(++iterations <= maxIterations,
                       "lower critical price: max iterations exceeded");
            Sv = nextCriticalPrice(Sv, yi, di);

            bs = bsCalculator(Sv, Option::Put);
            pi = bs.value();
            dc = bs.delta();

            yi = pi - A + Sv - X1;
            di = dc + 1;
        }
        return Sv;
    }

    Real AnalyticHolderExtensibleOptionEngine::I2Put() const {
        Real Sv = strike();
        Real A = arguments_.premium;
        if(A==0){
            return std::numeric_limits<Real>::infinity();
        }
        else{
            BlackScholesCalculator bs = bsCalculator(Sv, Option::Put);
            Real pi = bs.value();
            Real dc = bs.delta();

            Real yi = pi - A;
            //da/ds = 0
            Real di = dc - 0;
            Real epsilon = 0.001;

            //Newton-Raphson prosess
            Size iterations = 0;
            while (std::fabs(yi) > epsilon || !std::isfinite(yi)) {
                QL_REQUIRE(++iterations <= maxIterations,
                           "upper critical price: max iterations exceeded");
                Sv = nextCriticalPrice(Sv, yi, di);

                bs = bsCalculator(Sv, Option::Put);
                pi = bs.value();
                dc = bs.delta();

                yi = pi - A;
                di = dc - 0;
            }
            return Sv;
        }
    }


    BlackScholesCalculator AnalyticHolderExtensibleOptionEngine::bsCalculator(
                                    Real spot, Option::Type optionType) const {
        //Real spot = process_->x0();
        Real vol;
        DiscountFactor growth;
        DiscountFactor discount;
        Real X2 = arguments_.secondStrike;
        Time T2 = secondExpiryTime();
        Time t1 = firstExpiryTime();
        Time t = T2 - t1;

        //payoff
        ext::shared_ptr<PlainVanillaPayoff > vanillaPayoff =
            ext::make_shared<PlainVanillaPayoff>(optionType, X2);

        //QuantLib requires sigma * sqrt(T) rather than just sigma/volatility
        vol = volatility() * std::sqrt(t);
        //calculate dividend discount factor assuming continuous compounding (e^-rt)
        growth = dividendDiscount(t);
        //calculate payoff discount factor assuming continuous compounding
        discount = riskFreeDiscount(t);

        BlackScholesCalculator bs(vanillaPayoff, spot, growth, vol, discount);
        return bs;
    }

    Real AnalyticHolderExtensibleOptionEngine::M2(Real a, Real b, Real c, Real d, Real rho) const {
        BivariateCumulativeNormalDistributionDr78 CmlNormDist(rho);
        return CmlNormDist(b, d) - CmlNormDist(a, d) - CmlNormDist(b, c) + CmlNormDist(a,c);
    }

    Real AnalyticHolderExtensibleOptionEngine::N2(Real a, Real b) const {
        CumulativeNormalDistribution  NormDist;
        return NormDist(b) - NormDist(a);
    }

    Real AnalyticHolderExtensibleOptionEngine::strike() const {
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");
        return payoff->strike();
    }

    Time AnalyticHolderExtensibleOptionEngine::firstExpiryTime() const {
        return process_->time(arguments_.exercise->lastDate());
    }

    Time AnalyticHolderExtensibleOptionEngine::secondExpiryTime() const {
        return process_->time(arguments_.secondExpiryDate);
    }

    Volatility AnalyticHolderExtensibleOptionEngine::volatility() const {
        return process_->blackVolatility()->blackVol(firstExpiryTime(), strike());
    }
    Rate AnalyticHolderExtensibleOptionEngine::riskFreeRate() const {
        return process_->riskFreeRate()->zeroRate(firstExpiryTime(), Continuous,
            NoFrequency);
    }
    Rate AnalyticHolderExtensibleOptionEngine::dividendYield() const {
        return process_->dividendYield()->zeroRate(firstExpiryTime(),
            Continuous, NoFrequency);
    }

    DiscountFactor AnalyticHolderExtensibleOptionEngine::dividendDiscount(Time t) const {
        return process_->dividendYield()->discount(t);
    }

    DiscountFactor AnalyticHolderExtensibleOptionEngine::riskFreeDiscount(Time t) const {
        return process_->riskFreeRate()->discount(t);
    }

    Real AnalyticHolderExtensibleOptionEngine::y1(Option::Type type) const {
        Real S = process_->x0();
        Real I2 = (type == Option::Call) ? I2Call() : I2Put();

        Real b = riskFreeRate() - dividendYield();
        Real vol = volatility();
        Time t1 = firstExpiryTime();

        return (log(S / I2) + (b + squared(vol) / 2)*t1) / (vol*sqrt(t1));
    }

    Real AnalyticHolderExtensibleOptionEngine::y2(Option::Type type) const {
        Real S = process_->x0();
        Real I1 = (type == Option::Call) ? I1Call() : I1Put();

        Real b = riskFreeRate() - dividendYield();
        Real vol = volatility();
        Time t1 = firstExpiryTime();

        return (log(S / I1) + (b + squared(vol) / 2)*t1) / (vol*sqrt(t1));
    }

    Real AnalyticHolderExtensibleOptionEngine::z1() const {
        Real S = process_->x0();
        Real X2 = arguments_.secondStrike;
        Real b = riskFreeRate() - dividendYield();
        Real vol = volatility();
        Time T2 = secondExpiryTime();

        return (log(S / X2) + (b + squared(vol) / 2)*T2) / (vol*sqrt(T2));
    }

    Real AnalyticHolderExtensibleOptionEngine::z2() const {
        Real S = process_->x0();
        Real X1 = strike();

        Real b = riskFreeRate() - dividendYield();
        Real vol = volatility();
        Time t1 = firstExpiryTime();

        return (log(S / X1) + (b + squared(vol) / 2)*t1) / (vol*sqrt(t1));
    }

}
