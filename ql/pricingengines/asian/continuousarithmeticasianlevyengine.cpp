/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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
#include <ql/pricingengines/asian/continuousarithmeticasianlevyengine.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <cmath>
#include <utility>

using namespace std;

namespace QuantLib {

    namespace {

        Real exponentialIntegral(Real x) {
            return x == 0.0 ? Real(1.0) : Real(std::expm1(x) / x);
        }

        // 2 int_0^1 int_0^t exp(x*(s+t))*expm1(w*s) ds dt.
        Real levyVariance(Real x, Real w) {
            Size doublings = 0;
            while (std::fabs(x) + w > 0.5) {
                x *= 0.5;
                w *= 0.5;
                ++doublings;
            }

            // For phi(x) = expm1(x)/x the integral is the divided difference
            // 2*w*phi[x,2*x,2*x+w]. Expand its complete homogeneous polynomials
            // directly, without subtracting moments. All nodes have |x| <= 1;
            // 20 terms leave a truncation error below machine precision.
            // The auxiliary covariance C(x,w) = phi(x+w) - phi(x) supplies
            // the cross-interval contribution when doubling the interval.
            Real covariance = 0.0, variance = 0.0;
            Real power = 1.0, covariancePolynomial = 1.0;
            Real momentPolynomial = 1.0, variancePolynomial = 1.0;
            Real factorial = 2.0;
            for (Size k = 0; k < 20; ++k) {
                covariance += covariancePolynomial / factorial;
                variance += variancePolynomial / (factorial * (k + 3));
                power *= x;
                covariancePolynomial = (x + w) * covariancePolynomial + power;
                momentPolynomial = 2.0 * x * momentPolynomial + power;
                variancePolynomial = (2.0 * x + w) * variancePolynomial + momentPolynomial;
                factorial *= k + 3;
            }
            covariance *= w;
            variance *= 2.0 * w;

            // Restore the interval by splitting its covariance integral into
            // two triangles and a rectangle. This preserves small positive variance
            // without subtracting nearly equal moments, including for negative carry.
            for (Size i = 0; i < doublings; ++i) {
                const Real mean = exponentialIntegral(x);
                const Real e = std::exp(x), ew = std::exp(w);
                const Real em1 = std::expm1(w);
                variance = 0.25 * ((1.0 + e * e * ew) * variance +
                                   e * e * em1 * mean * mean + 2.0 * e * mean * covariance);
                covariance = 0.5 * ((1.0 + e * ew) * covariance + e * em1 * mean);
                x *= 2.0;
                w *= 2.0;
            }
            return variance;
        }

    }

    ContinuousArithmeticAsianLevyEngine::ContinuousArithmeticAsianLevyEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        Handle<Quote> currentAverage)
    : process_(std::move(process)), currentAverage_(std::move(currentAverage)) {
        registerWith(process_);
        registerWith(currentAverage_);
    }

    ContinuousArithmeticAsianLevyEngine::ContinuousArithmeticAsianLevyEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        Handle<Quote> currentAverage,
        Date startDate)
    : process_(std::move(process)), currentAverage_(std::move(currentAverage)),
      startDate_(startDate) {
        registerWith(process_);
        registerWith(currentAverage_);
    }

    void ContinuousArithmeticAsianLevyEngine::calculate() const {
        QL_REQUIRE(arguments_.averageType == Average::Arithmetic,
                   "not an Arithmetic average option");
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        // Prefer start date from option if available, otherwise use constructor parameter.
        // At least one must be specified.
        Date startDate = (arguments_.startDate != Date()) ? arguments_.startDate : startDate_;
        QL_REQUIRE(startDate != Date(), "start date not provided");
        QL_REQUIRE(startDate <= process_->riskFreeRate()->referenceDate(),
                   "start date must be earlier than or equal to reference date");

        DayCounter rfdc  = process_->riskFreeRate()->dayCounter();
        DayCounter divdc = process_->dividendYield()->dayCounter();
        DayCounter voldc = process_->blackVolatility()->dayCounter();
        Real spot = process_->stateVariable()->value();

        // payoff
        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        // original time to maturity
        Date maturity = arguments_.exercise->lastDate();
        Time T = rfdc.yearFraction(startDate,
                                   arguments_.exercise->lastDate());
        // remaining time to maturity
        Time T2 = rfdc.yearFraction(process_->riskFreeRate()->referenceDate(),
                                    arguments_.exercise->lastDate());

        Real strike = payoff->strike();

        Volatility volatility =
            process_->blackVolatility()->blackVol(maturity, strike);

        CumulativeNormalDistribution N;

        Rate riskFreeRate = process_->riskFreeRate()->
            zeroRate(maturity, rfdc, Continuous, NoFrequency);
        Rate dividendYield = process_->dividendYield()->
            zeroRate(maturity, divdc, Continuous, NoFrequency);
        Real b = riskFreeRate - dividendYield;

        const Real meanFactor = exponentialIntegral(b * T2);
        const Real mean = spot * T2 / T * meanFactor;
        const DiscountFactor discount = std::exp(-riskFreeRate * T2);
        const Real Se = mean * discount;

        Real X;
        if (T2 < T) {
            QL_REQUIRE(!currentAverage_.empty() && currentAverage_->isValid(),
                       "current average required for seasoned option");
            X = strike - ((T-T2)/T)*currentAverage_->value();
        } else {
            X = strike;
        }

        const Real w = volatility * volatility * T2;
        if (w == 0.0 || X <= 0.0) {
            results_.value = std::max(Real(payoff->optionType()) * (Se - X * discount), 0.0);
            return;
        }

        const Real V = std::log1p(levyVariance(b * T2, w) / (meanFactor * meanFactor));
        const Real stdDev = std::sqrt(V);
        const Real d1 = std::log(mean / X) / stdDev + 0.5 * stdDev;
        const Real d2 = d1 - stdDev;

        if(payoff->optionType()==Option::Call)
            results_.value = Se*N(d1) - X*exp(-riskFreeRate*T2)*N(d2);
        else
            results_.value = Se*N(d1) - X*exp(-riskFreeRate*T2)*N(d2)
                             - Se + X*exp(-riskFreeRate*T2);
    }

}
