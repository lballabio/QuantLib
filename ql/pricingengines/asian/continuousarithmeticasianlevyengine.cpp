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
#include <ql/quotes/simplequote.hpp>
#include <utility>

using namespace std;

namespace QuantLib {

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

        // Prefer arguments from option if available, otherwise use constructor parameters
        Date startDate = (arguments_.startDate != Date()) ? arguments_.startDate : startDate_;
        Handle<Quote> currentAverage = (!currentAverage_.empty() && currentAverage_->isValid())
                                        ? currentAverage_
                                        : Handle<Quote>();

        // If arguments provide current average, use it
        if (arguments_.currentAverage != Null<Real>()) {
            currentAverage = Handle<Quote>(ext::make_shared<SimpleQuote>(arguments_.currentAverage));
        }

        QL_REQUIRE(startDate <= process_->riskFreeRate()->referenceDate(),
                   "startDate must be earlier than or equal to reference date");

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

        Real Se = (std::fabs(b) > 1000*QL_EPSILON) 
            ? Real((spot/(T*b))*(exp((b-riskFreeRate)*T2)-exp(-riskFreeRate*T2)))
            : Real(spot*T2/T * std::exp(-riskFreeRate*T2));

        Real X;
        if (T2 < T) {
            QL_REQUIRE(!currentAverage.empty() && currentAverage->isValid(),
                       "current average required for seasoned option");
            X = strike - ((T-T2)/T)*currentAverage->value();
        } else {
            X = strike;
        }

        Real m = (std::fabs(b) > 1000*QL_EPSILON) ? ((exp(b*T2)-1)/b) : T2;

        Real M = (2*spot*spot/(b+volatility*volatility)) *
            (((exp((2*b+volatility*volatility)*T2)-1)
              / (2*b+volatility*volatility))-m);

        Real D = M/(T*T);

        Real V = log(D)-2*(riskFreeRate*T2+log(Se));

        Real d1 = (1/sqrt(V))*((log(D)/2)-log(X));
        Real d2 = d1-sqrt(V);

        if(payoff->optionType()==Option::Call)
            results_.value = Se*N(d1) - X*exp(-riskFreeRate*T2)*N(d2);
        else
            results_.value = Se*N(d1) - X*exp(-riskFreeRate*T2)*N(d2)
                             - Se + X*exp(-riskFreeRate*T2);
    }

}
