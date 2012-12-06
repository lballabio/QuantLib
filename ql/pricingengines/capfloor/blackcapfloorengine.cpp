/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006, 2007 StatPro Italia srl

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

#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/time/calendars/nullcalendar.hpp>

namespace QuantLib {

    BlackCapFloorEngine::BlackCapFloorEngine(
                              const Handle<YieldTermStructure>& discountCurve,
                              Volatility v,
                              const DayCounter& dc,
                              Real displacement)
    : discountCurve_(discountCurve),
      vol_(boost::shared_ptr<OptionletVolatilityStructure>(new
          ConstantOptionletVolatility(0, NullCalendar(), Following, v, dc))),
      displacement_(displacement) {
        registerWith(discountCurve_);
    }

    BlackCapFloorEngine::BlackCapFloorEngine(
                              const Handle<YieldTermStructure>& discountCurve,
                              const Handle<Quote>& v,
                              const DayCounter& dc,
                              Real displacement)
    : discountCurve_(discountCurve),
      vol_(boost::shared_ptr<OptionletVolatilityStructure>(new
          ConstantOptionletVolatility(0, NullCalendar(), Following, v, dc))),
      displacement_(displacement) {
        registerWith(discountCurve_);
        registerWith(vol_);
    }

    BlackCapFloorEngine::BlackCapFloorEngine(
                       const Handle<YieldTermStructure>& discountCurve,
                       const Handle<OptionletVolatilityStructure>& volatility,
                       Real displacement)
    : discountCurve_(discountCurve), vol_(volatility),
      displacement_(displacement) {
        registerWith(discountCurve_);
        registerWith(vol_);
    }

    void BlackCapFloorEngine::calculate() const {
        Real value = 0.0;
        Real vega = 0.0;
        Size optionlets = arguments_.startDates.size();
        std::vector<Real> values(optionlets, 0.0);
        std::vector<Real> vegas(optionlets, 0.0);
        std::vector<Real> stdDevs(optionlets, 0.0);
        CapFloor::Type type = arguments_.type;
        Date today = vol_->referenceDate();
        Date settlement = discountCurve_->referenceDate();

        for (Size i=0; i<optionlets; ++i) {
            Date paymentDate = arguments_.endDates[i];
            // handling of settlementDate, npvDate and includeSettlementFlows
            // should be implemented.
            // For the time being just discard expired caplets
            if (paymentDate > settlement) {
                DiscountFactor d = arguments_.nominals[i] *
                                   arguments_.gearings[i] *
                                   discountCurve_->discount(paymentDate) *
                                   arguments_.accrualTimes[i];

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
                            forward, stdDevs[i], d, displacement_) * sqrtTime;
                    }
                    // include caplets with past fixing date
                    values[i] = blackFormula(Option::Call,
                        strike, forward, stdDevs[i], d, displacement_);
                }
                if (type == CapFloor::Floor || type == CapFloor::Collar) {
                    Rate strike = arguments_.floorRates[i];
                    Real floorletVega = 0.0;
                    if (sqrtTime>0.0) {
                        stdDevs[i] = std::sqrt(vol_->blackVariance(fixingDate,
                                                                   strike));
                        floorletVega = blackFormulaStdDevDerivative(strike,
                            forward, stdDevs[i], d, displacement_) * sqrtTime;
                    }
                    Real floorlet = blackFormula(Option::Put,
                        strike, forward, stdDevs[i], d, displacement_);
                    if (type == CapFloor::Floor) {
                        values[i] = floorlet;
                        vegas[i] = floorletVega;
                    } else {
                        // a collar is long a cap and short a floor
                        values[i] -= floorlet;
                        vegas[i] -= floorletVega;
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
        results_.additionalResults["optionletsAtmForward"] = arguments_.forwards;
        if (type != CapFloor::Collar)
            results_.additionalResults["optionletsStdDev"] = stdDevs;
    }

}
