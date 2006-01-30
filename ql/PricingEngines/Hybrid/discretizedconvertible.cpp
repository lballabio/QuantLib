/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Theo Boafo
 Copyright (C) 2006 StatPro Italia srl

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

#include <ql/Instruments/bond.hpp>
#include <ql/Processes/blackscholesprocess.hpp>
#include <ql/Instruments/dividendschedule.hpp>
#include <ql/Instruments/callabilityschedule.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/PricingEngines/Hybrid/discretizedconvertible.hpp>
#include <vector>

namespace QuantLib {

    DiscretizedConvertible::DiscretizedConvertible(
                               const ConvertibleBond::option::arguments& args)
    : arguments_(args) {

        dividendValues_ = Array(arguments_.dividends.size(), 0.0);

		boost::shared_ptr<BlackScholesProcess> process =
            boost::dynamic_pointer_cast<BlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");

        Date settlementDate = process->riskFreeRate()->referenceDate();
        for (Size i=0; i<arguments_.dividends.size(); i++) {
            if (arguments_.dividends[i]->date() >= settlementDate) {
				dividendValues_[i] =
                    arguments_.dividends[i]->amount() *
                    process->riskFreeRate()->discount(
                                             arguments_.dividends[i]->date());
            }
        }
    }

    void DiscretizedConvertible::reset(Size size) {

        // Set to bond redemption values
        values_ = Array(size, arguments_.redemption);

        // coupon amounts should be added when adjusting
		// values_ = Array(size, arguments_.cashFlows.back()->amount());

		conversionProbability_ = Array(size, 0.0);
		spreadAdjustedRate_ = Array(size, 0.0);

		boost::shared_ptr<BlackScholesProcess> process =
            boost::dynamic_pointer_cast<BlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");

        DayCounter rfdc  = process->riskFreeRate()->dayCounter();

        // this takes care of convertibility and conversion probabilities
        adjustValues();

        Real creditSpread = arguments_.creditSpread->value();

        Date exercise = arguments_.exercise->lastDate();

        Rate riskFreeRate =
            process->riskFreeRate()->zeroRate(exercise, rfdc,
                                              Continuous, NoFrequency);

        // Calculate blended discount rate to be used on roll back.
        for (Size j=0; j<values_.size(); j++) {
           spreadAdjustedRate_[j] =
               conversionProbability_[j] * riskFreeRate +
               (1-conversionProbability_[j])*(riskFreeRate + creditSpread);
        }
    }

    void DiscretizedConvertible::postAdjustValuesImpl() {

        Size i;

        for (i=0; i<arguments_.callabilityTimes.size(); i++) {
            if (isOnTime(arguments_.callabilityTimes[i]))
                applyCallability(i);
        }

        for (i=0; i<arguments_.couponTimes.size(); i++) {
            if (isOnTime(arguments_.couponTimes[i]))
                addCoupon(i);
        }

        // add dividends somehow

        switch (arguments_.exercise->type()) {
          case Exercise::American:
            if (time() <= arguments_.stoppingTimes[1] &&
                time() >= arguments_.stoppingTimes[0])
                applyConvertibility();
            break;
          case Exercise::European:
            if (isOnTime(arguments_.stoppingTimes[0]))
                applyConvertibility();
            break;
          case Exercise::Bermudan:
            for (i = 0; i<arguments_.stoppingTimes.size(); i++) {
                if (isOnTime(arguments_.stoppingTimes[i]))
                    applyConvertibility();
            }
            break;
          default:
            QL_FAIL("invalid option type");
        }
    }

    void DiscretizedConvertible::applyConvertibility() {
		Array grid = method()->grid(time());
        for (Size j=0; j<values_.size(); j++) {
            Real payoff = arguments_.conversionRatio*grid[j];
            if (values_[j] < payoff) {
                values_[j] = payoff;
                conversionProbability_[j] = 1.0;
            }
        }
    }

    void DiscretizedConvertible::applyCallability(Size i) {
        Size j;
        switch (arguments_.callabilityTypes[i]) {
          case Callability::Call:
            for (j=0; j<values_.size(); j++) {
                values_[j] = std::min(values_[j],
                                      arguments_.callabilityPrices[i]);
            }
            break;
          case Callability::Put:
            for (j=0; j<values_.size(); j++) {
                values_[j] = std::max(values_[j],
                                      arguments_.callabilityPrices[i]);
            }
            break;
          default:
            QL_FAIL("unknown callability type");
        }
    }

    void DiscretizedConvertible::addCoupon(Size i) {
        values_ += arguments_.couponAmounts[i];
    }

}

