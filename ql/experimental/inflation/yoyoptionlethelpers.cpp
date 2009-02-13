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

namespace QuantLib {

    namespace {
        void no_deletion(void*) {}
    }


    YoYOptionletHelper::YoYOptionletHelper(
                  const Handle<Quote>& price,
                  Real notional,
                  YoYInflationCapFloor::Type capFloorType,
                  Period &lag,
                  Natural fixingDays,
                  const boost::shared_ptr<YoYInflationIndex>& index,
                  Rate strike, Size n,
                  const boost::shared_ptr<YoYInflationCapFloorEngine> &pricer)
    : BootstrapHelper<YoYOptionletVolatilitySurface>(price),
      notional_(notional), capFloorType_(capFloorType), lag_(lag),
      fixingDays_(fixingDays), index_(index), strike_(strike), n_(n),
      pricer_(pricer) {

        // build the instrument to reprice (only need do this once)
        yoyCapFloor_ = boost::shared_ptr<YoYInflationCapFloor>(
                 new YoYInflationCapFloor(
                     MakeYoYInflationCapFloor(capFloorType_, lag_,
                                              fixingDays_, index_, strike_, n_)
                     .withNominal(notional)));
        // dates already build in lag of index/instrument
        // these are the dates of the values of the index
        // that fix the capfloor
        earliestDate_ = yoyCapFloor_->fixingDates().front();
        latestDate_ = yoyCapFloor_->fixingDates().back();
        // each reprice is resetting the inflation surf in the
        // pricer... so set the pricer
        yoyCapFloor_->setPricingEngine(pricer_);
        // haven't yet set the vol (term structure = surface)
    }


    Real YoYOptionletHelper::impliedQuote() const {
        yoyCapFloor_->recalculate();
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
              boost::shared_ptr<YoYOptionletVolatilitySurface>(v,no_deletion),
              own);
        // in this case all we need to do is reset the vol in the pricer
        // we must do it because the surface is a different one each time
        // i.e. the pointer (handle) changes, not just what it points to
        pricer_->setVolatility(volSurf);
    }

}

