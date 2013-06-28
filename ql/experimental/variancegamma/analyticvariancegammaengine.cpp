/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2010 Adrian O' Neill

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

#include <ql/experimental/variancegamma/analyticvariancegammaengine.hpp>
#include <ql/exercise.hpp>
#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/pricingengines/blackscholescalculator.hpp>
#include <ql/math/integrals/segmentintegral.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>

namespace QuantLib {

    namespace {

        class Integrand : std::unary_function<Real,Real> {
        public:
            Integrand(const boost::shared_ptr<StrikedTypePayoff>& payoff,
                Real s0, Real t, Real riskFreeDiscount, Real dividendDiscount,
                Real sigma, Real nu, Real theta)
                : payoff_(payoff), s0_(s0), t_(t), riskFreeDiscount_(riskFreeDiscount),
                    dividendDiscount_(dividendDiscount),
                    sigma_(sigma), nu_(nu), theta_(theta) {
                omega_ = std::log(1.0 - theta_ * nu_ - (sigma_ * sigma_ * nu_) / 2.0) / nu_;
                // We can precompute the denominator of the gamma pdf (does not depend on x)
                // shape = t_/nu_, scale = nu_
                GammaFunction gf;
                gammaDenom_ = std::exp(gf.logValue(t_ / nu_)) * std::pow(nu_, t_ / nu_);
            }

            Real operator()(Real x) const {
                // Compute adjusted black scholes price
                Real s0_adj = s0_ * std::exp(theta_ * x + omega_ * t_ + (sigma_ * sigma_ * x) / 2.0);
                Real vol_adj = sigma_ * std::sqrt(x / t_);
                vol_adj *= std::sqrt(t_);

                BlackScholesCalculator bs(payoff_, s0_adj, dividendDiscount_, vol_adj, riskFreeDiscount_);
                Real bsprice = bs.value();

                // Multiply by gamma distribution
                Real gamp = (std::pow(x, t_ / nu_ - 1.0) * std::exp(-x / nu_)) / gammaDenom_;
                Real result = bsprice * gamp;
                return result;
            }

        private:
            boost::shared_ptr<StrikedTypePayoff> payoff_;
            Real s0_;
            Real t_;
            Real riskFreeDiscount_;
            Real dividendDiscount_;
            Rate sigma_;
            Real nu_;
            Real theta_;
            Real omega_;
            Real gammaDenom_;
        };
    }

    VarianceGammaEngine::VarianceGammaEngine(
        const boost::shared_ptr<VarianceGammaProcess>& process)
        : process_(process) {
            registerWith(process_);
    }

    void VarianceGammaEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
            "not an European Option");

        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        DiscountFactor dividendDiscount =
            process_->dividendYield()->discount(
            arguments_.exercise->lastDate());
        DiscountFactor riskFreeDiscount =
            process_->riskFreeRate()->discount(arguments_.exercise->lastDate());

        DayCounter rfdc  = process_->riskFreeRate()->dayCounter();
        Time t = rfdc.yearFraction(process_->riskFreeRate()->referenceDate(),
            arguments_.exercise->lastDate());
    
        Integrand f(payoff,
            process_->x0(),
            t, riskFreeDiscount, dividendDiscount,
            process_->sigma(), process_->nu(), process_->theta());

        SimpsonIntegral integrator(1e-4, 5000);

        Real infinity = 15.0 * std::sqrt(process_->nu() * t);
        results_.value = integrator(f, 0, infinity);
    }

}

