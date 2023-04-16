/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Banca Profilo S.p.A.
 Copyright (C) 2006 StatPro Italia srl

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

#include <ql/pricingengines/capfloor/mchullwhiteengine.hpp>
#include <utility>

namespace QuantLib {

    namespace detail {

        HullWhiteCapFloorPricer::HullWhiteCapFloorPricer(const CapFloor::arguments& args,
                                                         std::shared_ptr<HullWhite> model,
                                                         Time forwardMeasureTime)
        : args_(args), model_(std::move(model)), forwardMeasureTime_(forwardMeasureTime) {
            endDiscount_ =
                model_->termStructure()->discount(forwardMeasureTime_);

            Date referenceDate = model_->termStructure()->referenceDate();
            DayCounter dayCounter = model_->termStructure()->dayCounter();

            startTimes_.resize(args.startDates.size());
            for (Size i=0; i<startTimes_.size(); ++i)
                startTimes_[i] = dayCounter.yearFraction(referenceDate,
                                                         args.startDates[i]);

            endTimes_.resize(args.endDates.size());
            for (Size i=0; i<endTimes_.size(); ++i)
                endTimes_[i] = dayCounter.yearFraction(referenceDate,
                                                       args.endDates[i]);

            fixingTimes_.resize(args.fixingDates.size());
            for (Size i=0; i<fixingTimes_.size(); ++i)
                fixingTimes_[i] = dayCounter.yearFraction(referenceDate,
                                                          args.fixingDates[i]);
        }

        Real HullWhiteCapFloorPricer::operator()(const Path& path) const {

            bool isCap = (args_.type == CapFloor::Cap);
            Real npv = 0.0;
            Time Tb = forwardMeasureTime_;

            Size pastFixings = 0;
            for (Size i = 0; i<fixingTimes_.size(); i++) {
                Time tau = args_.accrualTimes[i];
                Time start = startTimes_[i],
                     end = endTimes_[i],
                     fixing = fixingTimes_[i];
                if (end <= 0.0) {
                    // the fixing is in the past...
                    pastFixings++;
                    // ...and the caplet is expired; nothing more to do.
                } else {
                    Rate ri_1, ri_2, currentLibor;
                    if (fixing <= 0.0) {
                        // current caplet. The fixing is in the past...
                        pastFixings++;
                        // ...so it is determined.
                        currentLibor = args_.forwards[i];
                        // However, the short rate at caplet expiry is not.
                        ri_2 = path[i-pastFixings+2];
                    } else {
                        // future caplet. Everything is to be forecast.
                        // The number of past fixings is used as an offset
                        // to index into the path.
                        ri_1 = path[i-pastFixings+1];
                        ri_2 = path[i-pastFixings+2];

                        DiscountFactor d1 =
                            model_->discountBond(fixing, start, ri_1);
                        DiscountFactor d2 =
                            model_->discountBond(fixing, end, ri_1);
                        currentLibor = (d1/d2-1)/tau;
                    }

                    Real accrualFactor =
                        1.0/model_->discountBond(end, Tb, ri_2);

                    Rate strike = isCap?
                        args_.capRates[i] :
                        args_.floorRates[i];
                    Real payoff = isCap?
                        std::max(currentLibor - strike, 0.0) :
                    std::max(strike - currentLibor, 0.0);

                    npv += payoff * tau * args_.gearings[i] *
                           args_.nominals[i] * accrualFactor;
                }
            }
            npv *= endDiscount_;
            return  npv;
        }

    }

}

