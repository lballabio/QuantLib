/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/exercise.hpp>
#include <ql/pricingengines/exotic/analyticcomplexchooserengine.hpp>
#include <ql/math/distributions/bivariatenormaldistribution.hpp>
#include <utility>

using std::pow;
using std::log;
using std::exp;
using std::sqrt;

namespace QuantLib {

    AnalyticComplexChooserEngine::AnalyticComplexChooserEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void AnalyticComplexChooserEngine::calculate() const {
        Real S = process_->x0();
        Real b;
        Real v;
        Real Xc = arguments_.strikeCall;
        Real Xp = arguments_.strikePut;
        Time T = choosingTime();
        Time Tc = callMaturity() - T;
        Time Tp = putMaturity() - T;

        Real i = criticalValue();

        b = riskFreeRate(T) - dividendYield(T);
        v = volatility(T);
        Real d1 = (log(S / i) + (b + pow(v, 2) / 2)*T) / (v*sqrt(T));
        Real d2 = d1 - v*sqrt(T);

        b = riskFreeRate(T + Tc) - dividendYield(T + Tc);
        v = volatility(Tc);
        Real y1 = (log(S / Xc) + (b + pow(v, 2) / 2)*Tc) / (v*sqrt(Tc));

        b = riskFreeRate(T + Tp) - dividendYield(T + Tp);
        v = volatility(Tp);
        Real y2 = (log(S / Xp) + (b + pow(v, 2) / 2)*Tp) / (v*sqrt(Tp));

        Real rho1 = sqrt(T / Tc);
        Real rho2 = sqrt(T / Tp);
        b = riskFreeRate(T + Tc) - dividendYield(T + Tc);
        Real r = riskFreeRate(T + Tc);
        Real ComplexChooser = S * exp((b - r)*Tc) *  BivariateCumulativeNormalDistributionDr78(rho1)(d1, y1)
            - Xc * exp(-r*Tc)*BivariateCumulativeNormalDistributionDr78(rho1)(d2, y1 - v * sqrt(Tc)) ;
        b = riskFreeRate(T + Tp) - dividendYield(T + Tp);
        r = riskFreeRate(T + Tp);
        ComplexChooser -= S * exp((b - r)*Tp) * BivariateCumulativeNormalDistributionDr78(rho2)(-d1, -y2);
        ComplexChooser += Xp * exp(-r*Tp) * BivariateCumulativeNormalDistributionDr78(rho2)(-d2, -y2 + v * sqrt(Tp));

        results_.value = ComplexChooser;
    }

    BlackScholesCalculator AnalyticComplexChooserEngine::bsCalculator(
                                   Real spot, Option::Type optionType) const {
        Real vol;
        DiscountFactor growth;
        DiscountFactor discount;
        Time T = choosingTime();

        // payoff
        ext::shared_ptr<PlainVanillaPayoff > vanillaPayoff;
        if (optionType == Option::Call){
            //TC-T
            Time t=callMaturity()-2*T;
            vanillaPayoff = ext::make_shared<PlainVanillaPayoff>(
                                          Option::Call, strike(Option::Call));
            //QuantLib requires sigma * sqrt(t) rather than just sigma/volatility
            vol = volatility(t) * std::sqrt(t);
            growth = dividendDiscount(t);
            discount = riskFreeDiscount(t);
        } else{
            Time t=putMaturity()-2*T;
            vanillaPayoff = ext::make_shared<PlainVanillaPayoff>(
                                            Option::Put, strike(Option::Put));
            vol = volatility(t) * std::sqrt(t);
            growth = dividendDiscount(t);
            discount = riskFreeDiscount(t);
        }

        BlackScholesCalculator bs(vanillaPayoff, spot, growth, vol, discount);
        return bs;
    }

    Real AnalyticComplexChooserEngine::criticalValue() const{
        Real Sv = process_->x0();

        BlackScholesCalculator bs=bsCalculator(Sv,Option::Call);
        Real ci = bs.value();
        Real dc = bs.delta();

        bs=bsCalculator(Sv,Option::Put);
        Real Pi = bs.value();
        Real dp = bs.delta();

        Real yi = ci - Pi;
        Real di = dc - dp;
        Real epsilon = 0.001;

        //Newton-Raphson process
        while (std::fabs(yi) > epsilon){
            Sv = Sv - yi / di;

            bs=bsCalculator(Sv,Option::Call);
            ci = bs.value();
            dc = bs.delta();

            bs=bsCalculator(Sv,Option::Put);
            Pi = bs.value();
            dp = bs.delta();

            yi = ci - Pi;
            di = dc - dp;
        }
        return Sv;
    }


    Real AnalyticComplexChooserEngine::strike(Option::Type optionType) const {
        if (optionType == Option::Call)
            return arguments_.strikeCall;
        else
            return arguments_.strikePut;
    }

    Time AnalyticComplexChooserEngine::choosingTime() const {
        return process_->time(arguments_.choosingDate);
    }

    Time AnalyticComplexChooserEngine::putMaturity() const {
        return process_->time(arguments_.exercisePut->lastDate());
    }

    Time AnalyticComplexChooserEngine::callMaturity() const {
        return process_->time(arguments_.exerciseCall->lastDate());
    }

    Volatility AnalyticComplexChooserEngine::volatility(Time t) const {
        return process_->blackVolatility()->blackVol(t, arguments_.strikeCall);
    }

    Rate AnalyticComplexChooserEngine::dividendYield(Time t) const {
        return process_->dividendYield()->zeroRate(t, Continuous, NoFrequency);
    }

    DiscountFactor AnalyticComplexChooserEngine::dividendDiscount(Time t) const {
        return process_->dividendYield()->discount(t);
    }

    Rate AnalyticComplexChooserEngine::riskFreeRate(Time t) const {
        return process_->riskFreeRate()->zeroRate(t, Continuous, NoFrequency);
    }

    DiscountFactor AnalyticComplexChooserEngine::riskFreeDiscount(Time t) const {
        return process_->riskFreeRate()->discount(t);
    }

}
