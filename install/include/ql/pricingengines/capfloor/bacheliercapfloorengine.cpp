/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014, 2015 Michael von den Driesch
 Copyright (C) 2019 Wojciech Ślusarski

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

#include <ql/pricingengines/blackformula.hpp>
#include <ql/pricingengines/capfloor/bacheliercapfloorengine.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/termstructures/volatility/optionlet/strippedoptionletadapter.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <utility>

namespace QuantLib {

    BachelierCapFloorEngine::BachelierCapFloorEngine(Handle<YieldTermStructure> discountCurve,
                                                     Volatility v,
                                                     const DayCounter& dc)
    : discountCurve_(std::move(discountCurve)),
      vol_(ext::shared_ptr<OptionletVolatilityStructure>(
          new ConstantOptionletVolatility(0, NullCalendar(), Following, v, dc))) {
        registerWith(discountCurve_);
    }

    BachelierCapFloorEngine::BachelierCapFloorEngine(Handle<YieldTermStructure> discountCurve,
                                                     const Handle<Quote>& v,
                                                     const DayCounter& dc)
    : discountCurve_(std::move(discountCurve)),
      vol_(ext::shared_ptr<OptionletVolatilityStructure>(
          new ConstantOptionletVolatility(0, NullCalendar(), Following, v, dc))) {
        registerWith(discountCurve_);
        registerWith(vol_);
    }

    BachelierCapFloorEngine::BachelierCapFloorEngine(
        Handle<YieldTermStructure> discountCurve, Handle<OptionletVolatilityStructure> volatility)
    : discountCurve_(std::move(discountCurve)), vol_(std::move(volatility)) {
        QL_REQUIRE(vol_->volatilityType() == Normal,
                   "BachelierCapFloorEngine should only be used for vol "
                   "surfaces stripped with normal model. Options were stripped "
                   "with model "
                       << vol_->volatilityType());
        registerWith(discountCurve_);
        registerWith(vol_);
    }

    void BachelierCapFloorEngine::calculate() const {
        Real value = 0.0;
        Real vega = 0.0;
        Size optionlets = arguments_.startDates.size();
        std::vector<Real> values(optionlets, 0.0);
        std::vector<Real> deltas(optionlets, 0.0);
        std::vector<Real> vegas(optionlets, 0.0);
        std::vector<Real> stdDevs(optionlets, 0.0);
        std::vector<DiscountFactor> discountFactors(optionlets, 0.0);
        CapFloor::Type type = arguments_.type;
        Date today = vol_->referenceDate();
        Date settlement = discountCurve_->referenceDate();

        for (Size i=0; i<optionlets; ++i) {
            Date paymentDate = arguments_.endDates[i];
            // handling of settlementDate, npvDate and includeSettlementFlows
            // should be implemented.
            // For the time being just discard expired caplets
            if (paymentDate > settlement) {
                DiscountFactor d = discountCurve_->discount(paymentDate);
                discountFactors[i] = d;
                Real accrualFactor = arguments_.nominals[i] *
                                   arguments_.gearings[i] *
                                   arguments_.accrualTimes[i];
                Real discountedAccrual = d * accrualFactor;

                Rate forward = arguments_.forwards[i];

                Date fixingDate = arguments_.fixingDates[i];
                Time sqrtTime = 0.0;
                if (fixingDate > today)
                    sqrtTime = std::sqrt(vol_->timeFromReference(fixingDate));

                if (type == CapFloor::Cap || type == CapFloor::Collar) {
                    Rate strike = arguments_.capRates[i];
                    if (sqrtTime>0.0) {
                        stdDevs[i] = std::sqrt(vol_->blackVariance(fixingDate,
                                                                   strike));
                        vegas[i] = bachelierBlackFormulaStdDevDerivative(strike,
                            forward, stdDevs[i], discountedAccrual) * sqrtTime;
                        deltas[i] = bachelierBlackFormulaAssetItmProbability(Option::Call,
                            strike, forward, stdDevs[i]);
                    }
                    // include caplets with past fixing date
                    values[i] = bachelierBlackFormula(Option::Call,
                        strike, forward, stdDevs[i], discountedAccrual);
                }
                if (type == CapFloor::Floor || type == CapFloor::Collar) {
                    Rate strike = arguments_.floorRates[i];
                    Real floorletVega = 0.0;
                    Real floorletDelta = 0.0;
                    if (sqrtTime>0.0) {
                        stdDevs[i] = std::sqrt(vol_->blackVariance(fixingDate,
                                                                   strike));
                        floorletVega = bachelierBlackFormulaStdDevDerivative(strike,
                            forward, stdDevs[i], discountedAccrual) * sqrtTime;
                        floorletDelta = Integer(Option::Put) * bachelierBlackFormulaAssetItmProbability(
                                                        Option::Put, strike, forward, 
                                                        stdDevs[i]);
                    }
                    Real floorlet = bachelierBlackFormula(Option::Put,
                        strike, forward, stdDevs[i], discountedAccrual);
                    if (type == CapFloor::Floor) {
                        values[i] = floorlet;
                        vegas[i] = floorletVega;
                        deltas[i] = floorletDelta;
                    } else {
                        // a collar is long a cap and short a floor
                        values[i] -= floorlet;
                        vegas[i] -= floorletVega;
                        deltas[i] -= floorletDelta;
                    }
                }
                value += values[i];
                vega += vegas[i];
            }
        }
        results_.value = value;
        results_.additionalResults["vega"] = vega;

        results_.additionalResults["optionletsPrice"] = values;
        results_.additionalResults["optionletsVega"] = vegas;
        results_.additionalResults["optionletsDelta"] = deltas;
        results_.additionalResults["optionletsDiscountFactor"] = discountFactors;
        results_.additionalResults["optionletsAtmForward"] = arguments_.forwards;
        if (type != CapFloor::Collar)
            results_.additionalResults["optionletsStdDev"] = stdDevs;
    }

}
