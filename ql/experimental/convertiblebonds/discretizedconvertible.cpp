/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Theo Boafo
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

#include <ql/experimental/convertiblebonds/discretizedconvertible.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/comparison.hpp>

namespace QuantLib {

    DiscretizedConvertible::DiscretizedConvertible(
             const ConvertibleBond::option::arguments& args,
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             const TimeGrid& grid)
    : arguments_(args), process_(process) {

        dividendValues_ = Array(arguments_.dividends.size(), 0.0);

        Date settlementDate = process_->riskFreeRate()->referenceDate();
        for (Size i=0; i<arguments_.dividends.size(); i++) {
            if (arguments_.dividends[i]->date() >= settlementDate) {
                dividendValues_[i] =
                    arguments_.dividends[i]->amount() *
                    process_->riskFreeRate()->discount(
                                             arguments_.dividends[i]->date());
            }
        }

        DayCounter dayCounter = process_->riskFreeRate()->dayCounter();
        Date bondSettlement = arguments_.settlementDate;

        stoppingTimes_.resize(arguments_.exercise->dates().size());
        for (Size i=0; i<stoppingTimes_.size(); ++i)
            stoppingTimes_[i] =
                dayCounter.yearFraction(bondSettlement,
                                        arguments_.exercise->date(i));

        callabilityTimes_.resize(arguments_.callabilityDates.size());
        for (Size i=0; i<callabilityTimes_.size(); ++i)
            callabilityTimes_[i] =
                dayCounter.yearFraction(bondSettlement,
                                        arguments_.callabilityDates[i]);

        couponTimes_.resize(arguments_.couponDates.size());
        for (Size i=0; i<couponTimes_.size(); ++i)
            couponTimes_[i] =
                dayCounter.yearFraction(bondSettlement,
                                        arguments_.couponDates[i]);

        dividendTimes_.resize(arguments_.dividendDates.size());
        for (Size i=0; i<dividendTimes_.size(); ++i)
            dividendTimes_[i] =
                dayCounter.yearFraction(bondSettlement,
                                        arguments_.dividendDates[i]);

        if (!grid.empty()) {
            // adjust times to grid
            for (Size i=0; i<stoppingTimes_.size(); i++)
                stoppingTimes_[i] = grid.closestTime(stoppingTimes_[i]);
            for (Size i=0; i<couponTimes_.size(); i++)
                couponTimes_[i] = grid.closestTime(couponTimes_[i]);
            for (Size i=0; i<callabilityTimes_.size(); i++)
                callabilityTimes_[i] = grid.closestTime(callabilityTimes_[i]);
            for (Size i=0; i<dividendTimes_.size(); i++)
                dividendTimes_[i] = grid.closestTime(dividendTimes_[i]);
        }
    }

    void DiscretizedConvertible::reset(Size size) {

        // Set to bond redemption values
        values_ = Array(size, arguments_.redemption);

        // coupon amounts should be added when adjusting
        // values_ = Array(size, arguments_.cashFlows.back()->amount());

        conversionProbability_ = Array(size, 0.0);
        spreadAdjustedRate_ = Array(size, 0.0);

        DayCounter rfdc  = process_->riskFreeRate()->dayCounter();

        // this takes care of convertibility and conversion probabilities
        adjustValues();

        Real creditSpread = arguments_.creditSpread->value();

        Date exercise = arguments_.exercise->lastDate();

        Rate riskFreeRate =
            process_->riskFreeRate()->zeroRate(exercise, rfdc,
                                               Continuous, NoFrequency);

        // Calculate blended discount rate to be used on roll back.
        for (Size j=0; j<values_.size(); j++) {
           spreadAdjustedRate_[j] =
               conversionProbability_[j] * riskFreeRate +
               (1-conversionProbability_[j])*(riskFreeRate + creditSpread);
        }
    }

    void DiscretizedConvertible::postAdjustValuesImpl() {

        bool convertible = false;
        switch (arguments_.exercise->type()) {
          case Exercise::American:
            if (time() <= stoppingTimes_[1] && time() >= stoppingTimes_[0])
                convertible = true;
            break;
          case Exercise::European:
            if (isOnTime(stoppingTimes_[0]))
                convertible = true;
            break;
          case Exercise::Bermudan:
            for (Size i=0; i<stoppingTimes_.size(); ++i) {
                if (isOnTime(stoppingTimes_[i]))
                    convertible = true;
            }
            break;
          default:
            QL_FAIL("invalid option type");
        }

        for (Size i=0; i<callabilityTimes_.size(); i++) {
            if (isOnTime(callabilityTimes_[i]))
                applyCallability(i,convertible);
        }

        for (Size i=0; i<couponTimes_.size(); i++) {
            if (isOnTime(couponTimes_[i]))
                addCoupon(i);
        }

        if (convertible)
            applyConvertibility();
    }

    void DiscretizedConvertible::applyConvertibility() {
        Array grid = adjustedGrid();
        for (Size j=0; j<values_.size(); j++) {
            Real payoff = arguments_.conversionRatio*grid[j];
            if (values_[j] <= payoff) {
                values_[j] = payoff;
                conversionProbability_[j] = 1.0;
            }
        }
    }

    void DiscretizedConvertible::applyCallability(Size i, bool convertible) {
        Size j;
        Array grid = adjustedGrid();
        switch (arguments_.callabilityTypes[i]) {
          case Callability::Call:
            if (arguments_.callabilityTriggers[i] != Null<Real>()) {
                Real conversionValue =
                    arguments_.redemption/arguments_.conversionRatio;
                Real trigger =
                    conversionValue*arguments_.callabilityTriggers[i];
                for (j=0; j<values_.size(); j++) {
                    // the callability is conditioned by the trigger...
                    if (grid[j] >= trigger) {
                        // ...and might trigger conversion
                        values_[j] =
                            std::min(std::max(
                                          arguments_.callabilityPrices[i],
                                          arguments_.conversionRatio*grid[j]),
                                     values_[j]);
                    }
                }
            } else if (convertible) {
                for (j=0; j<values_.size(); j++) {
                    // exercising the callability might trigger conversion
                    values_[j] =
                        std::min(std::max(arguments_.callabilityPrices[i],
                                          arguments_.conversionRatio*grid[j]),
                                 values_[j]);
                }
            } else {
                for (j=0; j<values_.size(); j++) {
                    values_[j] = std::min(arguments_.callabilityPrices[i],
                                          values_[j]);
                }
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

    Disposable<Array> DiscretizedConvertible::adjustedGrid() const {
        Time t = time();
        Array grid = method()->grid(t);
        // add back all dividend amounts in the future
        for (Size i=0; i<arguments_.dividends.size(); i++) {
            Time dividendTime = dividendTimes_[i];
            if (dividendTime >= t || close(dividendTime,t)) {
                const boost::shared_ptr<Dividend>& d = arguments_.dividends[i];
                DiscountFactor dividendDiscount =
                    process_->riskFreeRate()->discount(dividendTime) /
                    process_->riskFreeRate()->discount(t);
                for (Size j=0; j<grid.size(); j++)
                    grid[j] += d->amount(grid[j])*dividendDiscount;
            }
        }
        return grid;
    }

}

