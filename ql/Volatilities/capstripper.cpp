/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Francois du Vignaud

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

/*! \file capletvariancecurve.hpp
    \brief caplet variance curve
*/

#include <ql/Volatilities/capstripper.hpp>
#include <ql/Patterns/lazyobject.hpp>
#include <ql/capvolstructures.hpp>
#include <ql/types.hpp>
#include <ql/calendar.hpp>
#include <ql/daycounter.hpp>
#include <ql/quote.hpp>
#include <ql/Math/bilinearinterpolation.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/PricingEngines/CapFloor/blackcapfloorengine.hpp>
#include <ql/Indexes/xibor.hpp>
#include <ql/schedule.hpp>
#include <iostream>


namespace QuantLib {

    FloatingLeg LegHelper::makeLeg(const Period & startPeriod,
                                    const Period & endPeriod){
        Date startDate = referenceDate_ + startPeriod;
        Date endDate = referenceDate_ + endPeriod;
        Schedule schedule(startDate, endDate, index_->tenor(), calendar_,
                          convention_, convention_, true, false);
        return FloatingRateCouponVector(schedule, convention_, std::vector<Real>(1,1),
                                        fixingDays_, index_,
                                        std::vector<Real>(),
                                        std::vector<Spread>(),
                                        index_->dayCounter());
    };

    CapsStripper::CapsStripper(const Calendar & calendar,
                                BusinessDayConvention convention,
                                int fixingDays,
                                const std::vector<Period>& tenors,
                                const std::vector<Rate>& strikes,
                                const std::vector<std::vector<Handle<Quote> > >& volatilities,
                                const DayCounter& dayCounter,
                                const boost::shared_ptr<Xibor>& index,
                                const Handle< YieldTermStructure > yieldTermStructure)
        :CapletVolatilityStructure(0, calendar),
        dayCounter_(dayCounter),
        strikes_(strikes),
        tenorTimes_(tenors.size()),
        volatilities_(tenors.size(), strikes.size(),.1),
        marketDataPrices_(tenors.size(), strikes.size()),
        evaluationDate_(Settings::instance().evaluationDate())
        {
            // we convert tenors periods into times
            for (Size i = 0 ; i < tenors.size(); i++){
                Date tenorDate = 
                    calendar.advance(evaluationDate_, tenors[i], convention);
                tenorTimes_[i] = dayCounter_.yearFraction(evaluationDate_,tenorDate);
            }
            bilinearInterpolation_ = boost::shared_ptr<BilinearInterpolation>(
                   new BilinearInterpolation(tenorTimes_.begin(),tenorTimes_.end(),
                               strikes_.begin(), strikes_.end(), volatilities_));

           // we create the caps we will need later on
            LegHelper legHelper(evaluationDate_, calendar, fixingDays, convention, index);
            // market data
            marketDataCap.resize(tenors.size());
            for (Size i = 0 ; i < tenorTimes_.size(); i++){
               FloatingLeg floatingLeg = legHelper.makeLeg(index->tenor(), tenors[i]);
               marketDataCap[i].resize(strikes_.size());
               for (Size j = 0 ; j < strikes_.size(); j++){
                   boost::shared_ptr<PricingEngine> blackCapFloorEngine
                                             (new BlackCapFloorEngine(volatilities[i][j]));
                   marketDataCap[i][j] = boost::shared_ptr<CapFloor>(new Cap(floatingLeg,
                                             std::vector<Real>(1,strikes_[j]),
                                             yieldTermStructure, blackCapFloorEngine));
                   registerWith(marketDataCap[i][j]);
                   // we initialise the stripped volatilities with none stripped ones...
                   volatilities_[i][j] = volatilities[i][j]->value();
               }
            }
            // stripped Caps
            strippedCap.resize(tenors.size()-1);
            for (Size i = 0 ; i < strippedCap.size(); i++){
               FloatingLeg floatingLeg = legHelper.makeLeg(tenors[i],tenors[i+1]);
               strippedCap[i].resize(strikes_.size());
                for (Size j = 0 ; j < strikes_.size(); j++){
                   boost::shared_ptr<PricingEngine> blackCapFloorEngine
                                             (new BlackCapFloorEngine(volatilities[i][j]));
                   strippedCap[i][j] = boost::shared_ptr<CapFloor>(new Cap(floatingLeg,
                                             std::vector<Real>(1,strikes_[j]),
                                             yieldTermStructure, blackCapFloorEngine));
                }
            }
        // why is the return of floatingLeg() of CashFlow type ?
        maxDate_ = boost::dynamic_pointer_cast<FloatingRateCoupon>
                    (marketDataCap.back().front()->floatingLeg().back())->fixingDate(); 
    };

    void CapsStripper::performCalculations () const {

        Real accuracy = 1.0e-5;
        for (Size j = 0 ; j < strikes_.size(); j++) {
            Real previousCaplets = 0.0;
            for (Size i = 0 ; i < tenorTimes_.size()-1; i++) {
                Real capPrice = marketDataCap[i][j]->NPV();
                // if the cap price is too low we don't use stripped cap values
                if (capPrice<1.0e-4) {
                    volatilities_[i][j] = Null<Real>();
                    previousCaplets = 0.0;
                } else {
                    // we use a bigger cap instead and we set all stripped values equal to its implied volatilities
                    if (previousCaplets==0.0) {
                        Real vol = marketDataCap[i][j]->impliedVolatility(
                            capPrice, accuracy, 100);
                        for (Size k = 0; k<=i; ++k)
                            volatilities_[k][j] = vol;
                    } else {
                        Real capletsPrice = capPrice-previousCaplets;
                        volatilities_[i][j] = strippedCap[i][j]->impliedVolatility(
                            capletsPrice, accuracy, 100);
                    }
                    previousCaplets = capPrice;
                }
            }
        }
    };
}