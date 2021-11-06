/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Stamm
 Copyright (C) 2009 Jose Aparicio

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
#include <ql/experimental/credit/blackcdsoptionengine.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    BlackCdsOptionEngine::BlackCdsOptionEngine(Handle<DefaultProbabilityTermStructure> probability,
                                               Real recoveryRate,
                                               Handle<YieldTermStructure> termStructure,
                                               Handle<Quote> volatility)
    : probability_(std::move(probability)), recoveryRate_(recoveryRate),
      termStructure_(std::move(termStructure)), volatility_(std::move(volatility)) {

        registerWith(probability_);
        registerWith(termStructure_);
        registerWith(volatility_);
    }

    void BlackCdsOptionEngine::calculate() const {

        Date maturityDate = arguments_.swap->coupons().front()->date();
        Date exerciseDate = arguments_.exercise->date(0);
        QL_REQUIRE(maturityDate > exerciseDate,
                   "Underlying CDS should start after option maturity");
        Date settlement   = termStructure_->referenceDate();

        Rate spotFwdSpread = arguments_.swap->fairSpread();
        Rate swapSpread    = arguments_.swap->runningSpread();

        DayCounter tSDc = termStructure_->dayCounter();

        // The sense of the underlying/option has to be sent this way
        // to the Black formula, no sign.
        Real riskyAnnuity =
            std::fabs(arguments_.swap->couponLegNPV() / swapSpread);
        results_.riskyAnnuity = riskyAnnuity;

        Time T = tSDc.yearFraction(settlement, exerciseDate);

        Real stdDev = volatility_->value()  * std::sqrt(T);
        Option::Type callPut = (arguments_.side == Protection::Buyer) ?
                                                   Option::Call : Option::Put;

        results_.value =
            blackFormula(callPut, swapSpread, spotFwdSpread,
                         stdDev, riskyAnnuity);

        // if a non knock-out payer option, add front end protection value
        if (arguments_.side == Protection::Buyer && !arguments_.knocksOut) {
            Real frontEndProtection =
                static_cast<Real>(callPut) * arguments_.swap->notional()
                * (1.-recoveryRate_)
                * probability_->defaultProbability(exerciseDate)
                * termStructure_->discount(exerciseDate);
            results_.value += frontEndProtection;
        }
    }

    Handle<YieldTermStructure> BlackCdsOptionEngine::termStructure() {
        return termStructure_;
    }

    Handle<Quote> BlackCdsOptionEngine::volatility() {
        return volatility_;
    }

}
