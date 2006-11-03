/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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
#include <ql/Math/matrix.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/PricingEngines/CapFloor/blackcapfloorengine.hpp>
#include <ql/Indexes/xibor.hpp>
#include <ql/schedule.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Utilities/dataformatters.hpp>

namespace QuantLib {

    FloatingLeg LegHelper::makeLeg(const Period & startPeriod,
                                    const Period & endPeriod){
        Date startDate = referenceDate_ + startPeriod;
        Date endDate = referenceDate_ + endPeriod;
        Schedule schedule(startDate, endDate, index_->tenor(), calendar_,
                          convention_, convention_, true, false);
        return FloatingRateCouponVector(schedule,
                                        convention_,
                                        std::vector<Real>(1,1),
                                        fixingDays_, index_,
                                        std::vector<Real>(),
                                        std::vector<Spread>(),
                                        index_->dayCounter());
    }

    CapsStripper::CapsStripper(
        const Calendar & calendar,
        BusinessDayConvention convention,
        Integer fixingDays,
        const std::vector<Period>& tenors,
        const std::vector<Rate>& strikes,
        const std::vector<std::vector<Handle<Quote> > >& vols,
        const DayCounter& volatilityDayCounter,
        const boost::shared_ptr<Xibor>& index,
        const Handle< YieldTermStructure > termStructure,
        Real impliedVolatilityAccuracy)
    : CapletVolatilityStructure(0, calendar),
      volatilityDayCounter_(volatilityDayCounter),
      evaluationDate_(Settings::instance().evaluationDate()),
      tenors_(tenors), tenorTimes_(tenors.size()),
      strikes_(strikes),
      volatilities_(tenors.size(), strikes.size()),
      impliedVolatilityAccuracy_(impliedVolatilityAccuracy){

        QL_REQUIRE(vols.size()==tenors.size(),
                   "mismatch between tenors(" << tenors.size() <<
                   ") and vol rows(" << vols.size() << ")");
        QL_REQUIRE(vols[0].size()==strikes.size(),
                   "mismatch between strikes(" << strikes.size() <<
                   ") and vol columns(" << vols[0].size() << ")");

        // we create the caps we will need later on
        LegHelper legHelper(evaluationDate_, calendar, fixingDays,
            convention, index);
        // market data
        marketDataCap_.resize(tenors.size());
        for (Size i = 0 ; i < tenorTimes_.size(); i++) {
           FloatingLeg floatingLeg = legHelper.makeLeg(index->tenor(),
               tenors[i]);
           marketDataCap_[i].resize(strikes_.size());
           for (Size j = 0 ; j < strikes_.size(); j++) {
               boost::shared_ptr<PricingEngine> blackCapFloorEngine(new
                   BlackCapFloorEngine(vols[i][j], volatilityDayCounter));
               marketDataCap_[i][j] = boost::shared_ptr<Cap>(new
                   Cap(floatingLeg, std::vector<Real>(1,strikes_[j]),
                       termStructure, blackCapFloorEngine));
               registerWith(marketDataCap_[i][j]);
           }
        }
        // stripped Caps
        boost::shared_ptr<SimpleQuote> dummyQuote(new SimpleQuote(0.0));
        Handle<Quote> dummyQuoteHandle(dummyQuote);
        boost::shared_ptr<PricingEngine> blackCapFloorEngine(new
                   BlackCapFloorEngine(dummyQuoteHandle, volatilityDayCounter));
        strippedCap_.resize(tenors.size()-1);
        for (Size i = 0 ; i < strippedCap_.size(); i++) {
           FloatingLeg floatingLeg = legHelper.makeLeg(tenors[i],tenors[i+1]);
           strippedCap_[i].resize(strikes_.size());
            for (Size j = 0 ; j < strikes_.size(); j++) {
                strippedCap_[i][j] = boost::shared_ptr<Cap>(new
                   Cap(floatingLeg, std::vector<Real>(1,strikes_[j]),
                       termStructure, blackCapFloorEngine));
            }
        }

        // we store the times for which the volatility will be known
        for (Size i = 0 ; i < tenors.size(); i++){
            Date tenorDate = marketDataCap_[i][0]->lastFixingDate();
            tenorTimes_[i] = volatilityDayCounter_.yearFraction(evaluationDate_,tenorDate);
        }

        const std::vector<boost::shared_ptr<Cap> >& lastCapFloorRow =
            marketDataCap_.back();
        boost::shared_ptr<CapFloor> lastCap = lastCapFloorRow.front();
        boost::shared_ptr<CashFlow> lastCoupon(lastCap->floatingLeg().back());
        boost::shared_ptr<FloatingRateCoupon> lastFloatingCoupon =
            boost::dynamic_pointer_cast<FloatingRateCoupon>(lastCoupon);
        maxDate_ = lastFloatingCoupon->fixingDate();
        minStrike_ = strikes_.front();
        maxStrike_ = strikes_.back();
    }


    void CapsStripper::performCalculations () const {
        static const Real vegaThreshold = 1e-4;
        for (Size j = 0 ; j < strikes_.size(); j++){
            Real previousCaplets = 0.0;
            bool capVegaIsBigEnough = false;
            for (Size i = 0 ; i < tenorTimes_.size(); i++) {
                CapFloor & mktCap = *marketDataCap_[i][j];
                Real capPrice = mktCap.NPV();
                if (!capVegaIsBigEnough){
                    Real vega = mktCap.vega();
                    capVegaIsBigEnough = vega > vegaThreshold;
                    if (capVegaIsBigEnough){
                        Real vol = mktCap.impliedVolatility(
                            capPrice, impliedVolatilityAccuracy_, 10000);
                        for (Size k = 0; k<=i; ++k)
                            volatilities_[k][j] = vol;
                        previousCaplets = capPrice;
                    }
                } else {
                   Real capletsPrice = capPrice - previousCaplets;
                        volatilities_[i][j] = strippedCap_[i-1][j]->impliedVolatility(
                            capletsPrice, impliedVolatilityAccuracy_, 1000);
                   previousCaplets = capPrice;
                }

            }
             QL_REQUIRE(capVegaIsBigEnough,
                        "Unable to bootstrap Caps volatilities ! For each "
                        "strike there must be at least one cap for which the "
                        "vega is  superior to " << vegaThreshold <<
                        ", this is not the case for the strike: " <<
                        io::rate(strikes_[j]));
        }
    }

    Size locateTime(Time x,
                     const std::vector<Time>& values){
        if (x <= values[0])
            return 0;
        if (x >= values.back())
            return values.size()-1;
        Size i = 0;
        while (x > values[i])
            i++;
        return i;
    }

    Volatility CapsStripper::volatilityImpl(Time t, Rate r) const {
            calculate();
            Size timeIndex = locateTime(t, tenorTimes_);
            LinearInterpolation interpolator(strikes_.begin(), strikes_.end(),
                                    volatilities_[timeIndex]);
            return interpolator(r);
    }
}
