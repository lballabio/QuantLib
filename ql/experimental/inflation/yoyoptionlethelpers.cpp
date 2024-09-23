/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

#include <ql/experimental/inflation/yoyoptionlethelpers.hpp>
#include <ql/instruments/makeyoyinflationcapfloor.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {

    YoYOptionletHelper::YoYOptionletHelper(const Handle<Quote>& price,
                                           Real notional,
                                           YoYInflationCapFloor::Type capFloorType,
                                           Period& lag,
                                           DayCounter yoyDayCounter,
                                           Calendar paymentCalendar,
                                           Natural fixingDays,
                                           ext::shared_ptr<YoYInflationIndex> index,
                                           CPI::InterpolationType interpolation,
                                           Rate strike,
                                           Size n,
                                           ext::shared_ptr<YoYInflationCapFloorEngine> pricer)
    : BootstrapHelper<YoYOptionletVolatilitySurface>(price), notional_(notional),
      capFloorType_(capFloorType), lag_(lag), fixingDays_(fixingDays), index_(std::move(index)),
      strike_(strike), n_(n), yoyDayCounter_(std::move(yoyDayCounter)),
      calendar_(std::move(paymentCalendar)), pricer_(std::move(pricer)) {

        // build the instrument to reprice (only need do this once)
        yoyCapFloor_ =
            MakeYoYInflationCapFloor(capFloorType_, index_,
                                     n_, calendar_, lag_, interpolation)
            .withNominal(notional)
            .withFixingDays(fixingDays_)
            .withPaymentDayCounter(yoyDayCounter_)
            .withStrike(strike_);

        // dates already build in lag of index/instrument
        // these are the dates of the values of the index
        // that fix the capfloor
          earliestDate_ = ext::dynamic_pointer_cast<YoYInflationCoupon>(
              yoyCapFloor_->yoyLeg().front())->fixingDate();
          latestDate_ = ext::dynamic_pointer_cast<YoYInflationCoupon>(
              yoyCapFloor_->yoyLeg().back())->fixingDate();

        // each reprice is resetting the inflation surf in the
        // pricer... so set the pricer
        yoyCapFloor_->setPricingEngine(pricer_);
        // haven't yet set the vol (term structure = surface)
    }

    YoYOptionletHelper::YoYOptionletHelper(const Handle<Quote>& price,
                                           Real notional,
                                           YoYInflationCapFloor::Type capFloorType,
                                           Period& lag,
                                           DayCounter yoyDayCounter,
                                           Calendar paymentCalendar,
                                           Natural fixingDays,
                                           ext::shared_ptr<YoYInflationIndex> index,
                                           Rate strike,
                                           Size n,
                                           ext::shared_ptr<YoYInflationCapFloorEngine> pricer)
    : YoYOptionletHelper(price, notional, capFloorType, lag, yoyDayCounter, paymentCalendar,
                         fixingDays, index, CPI::AsIndex, strike, n, pricer) {}


    Real YoYOptionletHelper::impliedQuote() const {
        yoyCapFloor_->deepUpdate();
        return yoyCapFloor_->NPV();
    }


    void YoYOptionletHelper::setTermStructure(
                                           YoYOptionletVolatilitySurface* v) {

        BootstrapHelper<YoYOptionletVolatilitySurface>::setTermStructure(v);
        // set up a new yoyCapFloor
        // but this one does NOT own its inflation term structure
        const bool own = false;
        // create a handle to the new vol surface
        Handle<YoYOptionletVolatilitySurface> volSurf(
            ext::shared_ptr<YoYOptionletVolatilitySurface>(v, null_deleter()),
            own);
        // in this case all we need to do is reset the vol in the pricer
        // we must do it because the surface is a different one each time
        // i.e. the pointer (handle) changes, not just what it points to
        pricer_->setVolatility(volSurf);
    }

}

