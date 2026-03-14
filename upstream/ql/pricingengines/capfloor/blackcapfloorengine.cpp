/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006, 2007 StatPro Italia srl
 Copyright (C) 2015 Michael von den Driesch
 Copyright (C) 2019 Wojciech Ślusarski

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

#include <ql/pricingengines/blackformula.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <utility>

namespace QuantLib {

    BlackCapFloorEngine::BlackCapFloorEngine(Handle<YieldTermStructure> discountCurve,
                                             Volatility v,
                                             const DayCounter& dc,
                                             Real displacement)
    : discountCurve_(std::move(discountCurve)),
      vol_(ext::shared_ptr<OptionletVolatilityStructure>(
          new ConstantOptionletVolatility(0, NullCalendar(), Following, v, dc))),
      displacement_(displacement) {
        registerWith(discountCurve_);
    }

    BlackCapFloorEngine::BlackCapFloorEngine(Handle<YieldTermStructure> discountCurve,
                                             const Handle<Quote>& v,
                                             const DayCounter& dc,
                                             Real displacement)
    : discountCurve_(std::move(discountCurve)),
      vol_(ext::shared_ptr<OptionletVolatilityStructure>(
          new ConstantOptionletVolatility(0, NullCalendar(), Following, v, dc))),
      displacement_(displacement) {
        registerWith(discountCurve_);
        registerWith(vol_);
    }

    BlackCapFloorEngine::BlackCapFloorEngine(Handle<YieldTermStructure> discountCurve,
                                             Handle<OptionletVolatilityStructure> volatility,
                                             Real displacement)
    : discountCurve_(std::move(discountCurve)), vol_(std::move(volatility)) {
        QL_REQUIRE(
            vol_->volatilityType() == ShiftedLognormal,
            "BlackCapFloorEngine should only be used for vol surfaces stripped "
            "with shifted log normal model. Options were stripped with model "
                << vol_->volatilityType());
        if (displacement != Null< Real >()) {
            displacement_ = displacement;
            QL_REQUIRE(vol_->displacement() == displacement_,
                       "Displacement used for stripping and provided for "
                       "pricing differ. Model displacement was : "
                           << vol_->displacement());
        } else
            displacement_ = vol_->displacement();
        registerWith(discountCurve_);
        registerWith(vol_);
    }

    void BlackCapFloorEngine::calculate() const {
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
                        vegas[i] = blackFormulaStdDevDerivative(strike,
                            forward, stdDevs[i], discountedAccrual, displacement_) 
                            * sqrtTime;
                        deltas[i] = blackFormulaAssetItmProbability(Option::Call,
                            strike, forward, stdDevs[i], displacement_);
                    }
                    // include caplets with past fixing date
                    values[i] = blackFormula(Option::Call,
                        strike, forward, stdDevs[i], discountedAccrual, 
                        displacement_);
                }
                if (type == CapFloor::Floor || type == CapFloor::Collar) {
                    Rate strike = arguments_.floorRates[i];
                    Real floorletVega = 0.0;
                    Real floorletDelta = 0.0;
                    if (sqrtTime>0.0) {
                        stdDevs[i] = std::sqrt(vol_->blackVariance(fixingDate,
                                                                   strike));
                        floorletVega = blackFormulaStdDevDerivative(strike,
                            forward, stdDevs[i], discountedAccrual, displacement_) 
                            * sqrtTime;
                        floorletDelta = Integer(Option::Put) * blackFormulaAssetItmProbability(
                                                        Option::Put, strike, forward, 
                                                        stdDevs[i], displacement_);
                    }
                    Real floorlet = blackFormula(Option::Put,
                        strike, forward, stdDevs[i], discountedAccrual, displacement_);
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
