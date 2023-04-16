/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

#include <ql/pricingengines/capfloor/analyticcapfloorengine.hpp>
#include <optional>
#include <utility>

namespace QuantLib {

    AnalyticCapFloorEngine::AnalyticCapFloorEngine(const std::shared_ptr<AffineModel>& model,
                                                   Handle<YieldTermStructure> termStructure)
    : GenericModelEngine<AffineModel, CapFloor::arguments, CapFloor::results>(model),
      termStructure_(std::move(termStructure)) {
        registerWith(termStructure_);
    }


    void AnalyticCapFloorEngine::calculate() const {
        QL_REQUIRE(!model_.empty(), "null model");

        Date referenceDate;
        DayCounter dayCounter;

        std::shared_ptr<TermStructureConsistentModel> tsmodel =
            std::dynamic_pointer_cast<TermStructureConsistentModel>(*model_);
        if (tsmodel != nullptr) {
            referenceDate = tsmodel->termStructure()->referenceDate();
            dayCounter = tsmodel->termStructure()->dayCounter();
        } else {
            referenceDate = termStructure_->referenceDate();
            dayCounter = termStructure_->dayCounter();
        }

        Real value = 0.0;
        CapFloor::Type type = arguments_.type;
        Size nPeriods = arguments_.endDates.size();

        bool includeRefDatePayments =
            Settings::instance().includeReferenceDateEvents();
        if (referenceDate == Settings::instance().evaluationDate()) {
            std::optional<bool> includeTodaysPayments =
                Settings::instance().includeTodaysCashFlows();
            if (includeTodaysPayments) // NOLINT(readability-implicit-bool-conversion)
                includeRefDatePayments = *includeTodaysPayments;
        }

        for (Size i=0; i<nPeriods; i++) {

            Time fixingTime =
                dayCounter.yearFraction(referenceDate,
                                        arguments_.fixingDates[i]);
            Time paymentTime =
                dayCounter.yearFraction(referenceDate,
                                        arguments_.endDates[i]);

            bool not_expired =
                includeRefDatePayments ? paymentTime >= 0.0 : paymentTime > 0.0;

            if (not_expired) {
                Time tenor = arguments_.accrualTimes[i];
                Rate fixing = arguments_.forwards[i];
                if (fixingTime <= 0.0) {
                    if (type == CapFloor::Cap || type == CapFloor::Collar) {
                        DiscountFactor discount = model_->discount(paymentTime);
                        Rate strike = arguments_.capRates[i];
                        value += discount * arguments_.nominals[i] * tenor
                               * arguments_.gearings[i]
                               * std::max(0.0, fixing - strike);
                    }
                    if (type == CapFloor::Floor || type == CapFloor::Collar) {
                        DiscountFactor discount = model_->discount(paymentTime);
                        Rate strike = arguments_.floorRates[i];
                        Real mult = (type == CapFloor::Floor) ? 1.0 : -1.0;
                        value += discount * arguments_.nominals[i] * tenor
                               * mult * arguments_.gearings[i]
                               * std::max(0.0, strike - fixing);
                    }
                } else {
                    Time maturity =
                        dayCounter.yearFraction(referenceDate,
                                                arguments_.startDates[i]);
                    if (type == CapFloor::Cap || type == CapFloor::Collar) {
                        Real temp = 1.0+arguments_.capRates[i]*tenor;
                        value += arguments_.nominals[i] *
                            arguments_.gearings[i] * temp *
                            model_->discountBondOption(Option::Put, 1.0/temp,
                                                       maturity, paymentTime);
                    }
                    if (type == CapFloor::Floor || type == CapFloor::Collar) {
                        Real temp = 1.0+arguments_.floorRates[i]*tenor;
                        Real mult = (type == CapFloor::Floor) ? 1.0 : -1.0;
                        value += arguments_.nominals[i] *
                            arguments_.gearings[i] * temp * mult *
                            model_->discountBondOption(Option::Call, 1.0/temp,
                                                       maturity, paymentTime);
                    }
                }
            }
        }

        results_.value = value;
    }

}
