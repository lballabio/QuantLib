/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Klaus Spanderen

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

/*! \file analyticbsmhullwhiteengine.hpp
    \brief analytic Black-Scholes engines including stochastic interest rates
*/

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/voltermstructure.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analyticbsmhullwhiteengine.hpp>

namespace QuantLib {

    namespace {
        class ShiftedBlackVolTermStructure : public BlackVolTermStructure {
          public:
            ShiftedBlackVolTermStructure(
                Real varianceOffset,
                const Handle<BlackVolTermStructure> & volTS) 
                : BlackVolTermStructure(volTS->referenceDate(),
                                        volTS->calendar(),
                                        volTS->dayCounter()),
                  varianceOffset_(varianceOffset),
                  volTS_(volTS) { }

            Real minStrike() const { return volTS_->minStrike(); }
            Real maxStrike() const { return volTS_->maxStrike(); }
            Date maxDate() const   { return volTS_->maxDate(); }

          protected:
            Real blackVarianceImpl(Time t, Real strike) const {
                return volTS_->blackVariance(t, strike, true)+varianceOffset_;
            }
            Volatility blackVolImpl(Time t, Real strike) const {
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
                                  const Real equityShortRateCorrelation,
                                  const boost::shared_ptr<HullWhite> & model)
    : GenericModelEngine<HullWhite,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      rho_(equityShortRateCorrelation) {
    }

    void AnalyticBSMHullWhiteEngine::calculate() const {

        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");


        const boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        const boost::shared_ptr<Exercise> exercise = arguments_.exercise;
        
        Time t = process->riskFreeRate()->dayCounter().yearFraction(
           process->riskFreeRate()->referenceDate(), exercise->lastDate());

        const Real a = model_->params()[0];
        const Real sigma = model_->params()[1];
        const Real eta = 
            process->blackVolatility()->blackVol(exercise->lastDate(),
                                                 payoff->strike());
        
        const Real v = sigma*sigma/(a*a)
            *(t + 2/a*std::exp(-a*t) - 1/(2*a)*std::exp(-2*a*t) - 3/(2*a));
        const Real mu = 2*rho_*sigma*eta/a*(t-1/a*(1-std::exp(-a*t)));

        const Real varianceOffset = v + mu;

        Handle<BlackVolTermStructure> volTS(
             boost::shared_ptr<BlackVolTermStructure>(
                new ShiftedBlackVolTermStructure(varianceOffset,
                                                 process->blackVolatility())));

        const boost::shared_ptr<GeneralizedBlackScholesProcess> adjProcess(
                new GeneralizedBlackScholesProcess(process->stateVariable(),
                                                   process->dividendYield(),
                                                   process->riskFreeRate(),
                                                   volTS));

        const boost::shared_ptr<AnalyticEuropeanEngine> bsmEngine(
                                            new AnalyticEuropeanEngine());

        VanillaOption(adjProcess, payoff, exercise, bsmEngine).NPV();

        results_ = *dynamic_cast<const OneAssetOption::results*>(
                                                    bsmEngine->getResults());
    }
}
