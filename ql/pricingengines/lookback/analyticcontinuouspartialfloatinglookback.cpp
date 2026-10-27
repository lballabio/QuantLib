/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou
 Copyright (C) 2007 StatPro Italia srl

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
#include <ql/pricingengines/lookback/analyticcontinuouspartialfloatinglookback.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    AnalyticContinuousPartialFloatingLookbackEngine::
        AnalyticContinuousPartialFloatingLookbackEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void AnalyticContinuousPartialFloatingLookbackEngine::calculate() const {

        ext::shared_ptr<FloatingTypePayoff> payoff =
            ext::dynamic_pointer_cast<FloatingTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "Non-floating payoff given");

        QL_REQUIRE(process_->x0() > 0.0, "negative or null underlying");

        switch (payoff->optionType()) {
          case Option::Call:
            results_.value = A(1);
            break;
          case Option::Put:
            results_.value = A(-1);
            break;
          default:
            QL_FAIL("Unknown type");
        }
    }

    Real AnalyticContinuousPartialFloatingLookbackEngine::underlying() const {
        return process_->x0();
    }

    Time AnalyticContinuousPartialFloatingLookbackEngine::residualTime() const {
        return process_->time(arguments_.exercise->lastDate());
    }

    Volatility AnalyticContinuousPartialFloatingLookbackEngine::volatility() const {
        return process_->blackVolatility()->blackVol(residualTime(), minmax());
    }

    Real AnalyticContinuousPartialFloatingLookbackEngine::stdDeviation() const {
        return volatility() * std::sqrt(residualTime());
    }

    Rate AnalyticContinuousPartialFloatingLookbackEngine::riskFreeRate() const {
        return process_->riskFreeRate()->zeroRate(residualTime(), Continuous,
                                                  NoFrequency);
    }

    DiscountFactor AnalyticContinuousPartialFloatingLookbackEngine::riskFreeDiscount()
                                 const {
        return process_->riskFreeRate()->discount(residualTime());
    }

    Rate AnalyticContinuousPartialFloatingLookbackEngine::dividendYield() const {
        return process_->dividendYield()->zeroRate(residualTime(),
                                                   Continuous, NoFrequency);
    }

    DiscountFactor AnalyticContinuousPartialFloatingLookbackEngine::dividendDiscount()
                                 const {
        return process_->dividendYield()->discount(residualTime());
    }

    Real AnalyticContinuousPartialFloatingLookbackEngine::minmax() const {
        return arguments_.minmax;
    }

    Real AnalyticContinuousPartialFloatingLookbackEngine::lambda() const {
        return arguments_.lambda;
    }

    Time AnalyticContinuousPartialFloatingLookbackEngine::lookbackPeriodEndTime() const {
        return process_->time(arguments_.lookbackPeriodEnd);
    }


    Real AnalyticContinuousPartialFloatingLookbackEngine::A(Real eta) const {
        bool fullLookbackPeriod = lookbackPeriodEndTime() == residualTime();
        Real carry = riskFreeRate() - dividendYield();
        Volatility vol = volatility();
        Real x = 2.0*carry/(vol*vol);
        Real s = underlying()/minmax();

        Real ls = std::log(s);
        Real d1 = ls/stdDeviation() + 0.5*(x+1.0)*stdDeviation();
        Real d2 = d1 - stdDeviation();

        Real e1 = 0, e2 = 0;
        if (!fullLookbackPeriod)
        {
            e1 = (carry + vol * vol / 2) * (residualTime() - lookbackPeriodEndTime()) / (vol * std::sqrt(residualTime() - lookbackPeriodEndTime()));
            e2 = e1 - vol * std::sqrt(residualTime() - lookbackPeriodEndTime());
        }

        Real f1 = (ls + (carry + vol * vol / 2) * lookbackPeriodEndTime()) / (vol * std::sqrt(lookbackPeriodEndTime()));
        Real f2 = f1 - vol * std::sqrt(lookbackPeriodEndTime());

        Real l1 = std::log(lambda()) / vol;
        Real g1 = l1 / std::sqrt(residualTime());

        Real n1 = f_(eta*(d1 - g1));
        Real n2 = f_(eta*(d2 - g1));

        BivariateCumulativeNormalDistributionWe04DP cnbn1(1), cnbn2(0), cnbn3(-1);
        if (!fullLookbackPeriod) {
            cnbn1 = BivariateCumulativeNormalDistributionWe04DP (std::sqrt(lookbackPeriodEndTime() / residualTime()));
            cnbn2 = BivariateCumulativeNormalDistributionWe04DP (-std::sqrt(1 - lookbackPeriodEndTime() / residualTime()));
            cnbn3 = BivariateCumulativeNormalDistributionWe04DP (-std::sqrt(lookbackPeriodEndTime() / residualTime()));
        }

        Real n3 = cnbn1(eta*(-f1+2.0* carry * std::sqrt(lookbackPeriodEndTime()) / vol), eta*(-d1+x*stdDeviation()-g1));
        Real n4 = 0, n5 = 0, n6 = 0, n7 = 0;
        if (!fullLookbackPeriod)
        {
            Real g2 = l1 / std::sqrt(residualTime() - lookbackPeriodEndTime());
            n4 = cnbn2(-eta*(d1+g1), eta*(e1 + g2));
            n5 = cnbn2(-eta*(d1-g1), eta*(e1 - g2));
            n6 = cnbn3(eta*-f2, eta*(d2 - g1));
            n7 = f_(eta*(e2 - g2));
        }
        else
        {
            n4 = f_(-eta*(d1+g1));
        }

        Real n8 = f_(-eta*f1);

        if (std::fabs(x) < 1.0e-7) {
            // Apply l'Hopital's rule to the combined numerator of the 1/x terms.
            const auto derivative = [this](Real h, Real k, Real dh, Real dk, Real rho) {
                if (rho == 1.0)
                    return h < k ? f_.derivative(h) * dh : f_.derivative(k) * dk;
                if (rho == -1.0)
                    return h + k > 0.0 ? f_.derivative(h) * dh + f_.derivative(k) * dk
                                       : 0.0;
                const Real z = std::sqrt((1.0 - rho) * (1.0 + rho));
                return f_.derivative(h) * f_((k - rho * h) / z) * dh +
                       f_.derivative(k) * f_((h - rho * k) / z) * dk;
            };
            const Real a = stdDeviation();
            const Real u = vol * std::sqrt(lookbackPeriodEndTime());
            const Real v = vol * std::sqrt(residualTime() - lookbackPeriodEndTime());
            const Real dn3 = derivative(-eta * f1, eta * (-d1 - g1), eta * u / 2.0,
                                       eta * a / 2.0, u / a);
            Real dn4;
            if (fullLookbackPeriod) {
                dn4 = -eta * a / 2.0 * f_.derivative(d1 + g1);
            } else {
                const Real g2 = l1 / std::sqrt(residualTime() - lookbackPeriodEndTime());
                dn4 = derivative(-eta * (d1 + g1), eta * (e1 + g2), -eta * a / 2.0,
                                 eta * v / 2.0, -v / a);
            }
            Real ratio = -ls * n3 + dn3 - (a * a / 2.0 + std::log(lambda())) * n4 - dn4;
            if (!fullLookbackPeriod) {
                const Real g2 = l1 / std::sqrt(residualTime() - lookbackPeriodEndTime());
                const Real dn7 = eta * v / 2.0 * f_.derivative(e2 - g2);
                const Real dn8 = -eta * u / 2.0 * f_.derivative(f1);
                ratio -= u * u / 2.0 * n7 * n8 + dn7 * n8 + n7 * dn8;
            }
            return eta * riskFreeDiscount() *
                   (underlying() * n1 - lambda() * minmax() * n2 +
                    underlying() * (lambda() * (ratio - n7 * n8) + n5) +
                    lambda() * minmax() * n6);
        }

        Real pow_s = std::pow(s, -x);
        Real pow_l = std::pow(lambda(), x);

        if (!fullLookbackPeriod)
        {
            return eta*(underlying() * dividendDiscount() * n1 -
                        lambda() * minmax() * riskFreeDiscount() * n2 + 
                        underlying() * riskFreeDiscount() * lambda() / x *
                        (pow_s * n3 - dividendDiscount() / riskFreeDiscount() * pow_l * n4)
                        + underlying() * dividendDiscount() * n5 + 
                        riskFreeDiscount() * lambda() * minmax() * n6 -
                        std::exp(-carry * (residualTime() - lookbackPeriodEndTime())) * 
                        dividendDiscount() * (1 + 0.5 * vol * vol / carry) * lambda() * 
                        underlying() * n7 * n8);
        }
        else
        {
            //Simpler calculation
            return eta*(underlying() * dividendDiscount() * n1 -
                        lambda() * minmax() * riskFreeDiscount() * n2 + 
                        underlying() * riskFreeDiscount() * lambda() / x *
                        (pow_s * n3 - dividendDiscount() / riskFreeDiscount() * pow_l * n4));
        }
    }
}

