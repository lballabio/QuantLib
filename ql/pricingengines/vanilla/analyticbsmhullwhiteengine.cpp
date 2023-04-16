/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Klaus Spanderen

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

/*! \file analyticbsmhullwhiteengine.hpp
    \brief analytic Black-Scholes engines including stochastic interest rates
*/

#include <ql/pricingengines/vanilla/analyticbsmhullwhiteengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        class ShiftedBlackVolTermStructure : public BlackVolTermStructure {
          public:
            ShiftedBlackVolTermStructure(
                Real varianceOffset,
                const Handle<BlackVolTermStructure> & volTS)
                : BlackVolTermStructure(volTS->referenceDate(),
                                        volTS->calendar(),
                                        Following,
                                        volTS->dayCounter()),
                  varianceOffset_(varianceOffset),
                  volTS_(volTS) { }

            Real minStrike() const override { return volTS_->minStrike(); }
            Real maxStrike() const override { return volTS_->maxStrike(); }
            Date maxDate() const override { return volTS_->maxDate(); }

          protected:
            Real blackVarianceImpl(Time t, Real strike) const override {
                return volTS_->blackVariance(t, strike, true)+varianceOffset_;
            }
            Volatility blackVolImpl(Time t, Real strike) const override {
                Time nonZeroMaturity = (t==0.0 ? 0.00001 : t);
                Real var = blackVarianceImpl(nonZeroMaturity, strike);
                return std::sqrt(var/nonZeroMaturity);
            }

          private:
            const Real varianceOffset_;
            const Handle<BlackVolTermStructure> volTS_;
        };
    }

    AnalyticBSMHullWhiteEngine::AnalyticBSMHullWhiteEngine(
        Real equityShortRateCorrelation,
        std::shared_ptr<GeneralizedBlackScholesProcess> process,
        const std::shared_ptr<HullWhite>& model)
    : GenericModelEngine<HullWhite, VanillaOption::arguments, VanillaOption::results>(model),
      rho_(equityShortRateCorrelation), process_(std::move(process)) {
        QL_REQUIRE(process_, "no Black-Scholes process specified");
        QL_REQUIRE(!model_.empty(), "no Hull-White model specified");
        registerWith(process_);
    }

    void AnalyticBSMHullWhiteEngine::calculate() const {

        QL_REQUIRE(process_->x0() > 0.0, "negative or null underlying given");

        const std::shared_ptr<StrikedTypePayoff> payoff =
            std::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        const std::shared_ptr<Exercise> exercise = arguments_.exercise;

        Time t = process_->riskFreeRate()->dayCounter().yearFraction(
                                    process_->riskFreeRate()->referenceDate(),
                                    exercise->lastDate());

        const Real a = model_->params()[0];
        const Real sigma = model_->params()[1];
        const Real eta =
            process_->blackVolatility()->blackVol(exercise->lastDate(),
                                                  payoff->strike());

        Real varianceOffset;
        if (a*t > std::pow(QL_EPSILON, 0.25)) {
            const Real v = sigma*sigma/(a*a)
                *(t + 2/a*std::exp(-a*t) - 1/(2*a)*std::exp(-2*a*t) - 3/(2*a));
            const Real mu = 2*rho_*sigma*eta/a*(t-1/a*(1-std::exp(-a*t)));

            varianceOffset = v + mu;
        }
        else {
            // low-a algebraic limit
            const Real v = sigma*sigma*t*t*t*(1/3.0-0.25*a*t+7/60.0*a*a*t*t);
            const Real mu = rho_*sigma*eta*t*t*(1-a*t/3.0+a*a*t*t/12.0);

            varianceOffset = v + mu;
        }

        Handle<BlackVolTermStructure> volTS(
             std::shared_ptr<BlackVolTermStructure>(
              new ShiftedBlackVolTermStructure(varianceOffset,
                                               process_->blackVolatility())));

        std::shared_ptr<GeneralizedBlackScholesProcess> adjProcess(
                new GeneralizedBlackScholesProcess(process_->stateVariable(),
                                                   process_->dividendYield(),
                                                   process_->riskFreeRate(),
                                                   volTS));

        std::shared_ptr<AnalyticEuropeanEngine> bsmEngine(
                                      new AnalyticEuropeanEngine(adjProcess));

        VanillaOption(payoff, exercise).setupArguments(
                                                   bsmEngine->getArguments());
        bsmEngine->calculate();

        results_ = *dynamic_cast<const OneAssetOption::results*>(
                                                    bsmEngine->getResults());
    }
}
