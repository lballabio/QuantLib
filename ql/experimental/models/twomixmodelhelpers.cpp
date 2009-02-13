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

#include <ql/experimental/models/twomixmodelhelpers.hpp>
#include <ql/experimental/inflation/inflationcapfloorengines.hpp>
#include <ql/instruments/makecapfloor.hpp>

namespace QuantLib {

    TwoMixYoYModelHelper::TwoMixYoYModelHelper(
                           const Handle<Quote> &volatility,
                           boost::shared_ptr<PricingEngine> marketValueEngine,
                           Real notional,
                           YoYInflationCapFloor::Type capFloorType,
                           Period &lag, Natural fixingDays,
                           const boost::shared_ptr<YoYInflationIndex>& index,
                           Rate strike, Size n, Real normalizingFactor)
    : CalibrationHelper(
                   volatility,
                   index->yoyInflationTermStructure()->nominalTermStructure(),
                   false), // i.e. calibrateVolatility = false
      volatility_(volatility), notional_(notional), capFloorType_(capFloorType),
      lag_(lag), fixingDays_(fixingDays),
      index_(index), strike_(strike), n_(n),
      capFloor_( MakeYoYInflationCapFloor(capFloorType,
                                          lag, fixingDays, index, strike, n)
                 .withNominal(notional)
                 .asOptionlet() ),
      marketVol_(boost::shared_ptr<ConstantYoYOptionletVolatility>(
               new ConstantYoYOptionletVolatility(volatility->value(), lag))),
      marketValueEngine_(marketValueEngine),
      normalizingFactor_(normalizingFactor) {

        // set the volatility in the marketValueEngine_. Very, very,
        // awkward because of conversion between types and handle/ptr
        // etc so can get at desired methods etc
        boost::shared_ptr<YoYInflationCapFloorEngine> e =
            boost::dynamic_pointer_cast<YoYInflationCapFloorEngine>(
                                                          marketValueEngine_);
        Handle<YoYOptionletVolatilitySurface> sH(
             boost::dynamic_pointer_cast<YoYOptionletVolatilitySurface>
                                                 (marketVol_.currentLink()));
        e->setVolatility(sH);
        capFloor_->setPricingEngine(marketValueEngine_);
        marketValue_ = capFloor_->NPV();
    }


    Real TwoMixYoYModelHelper::modelValue() const {
        capFloor_->setPricingEngine(engine_);
        return capFloor_->NPV();
    }


    Real TwoMixYoYModelHelper::blackPrice(Volatility volatility) const {

        // set the vol and the engine to temporary values, then value
        // whilst saving the old one
        boost::shared_ptr<YoYInflationCapFloorEngine> e
            = boost::dynamic_pointer_cast<YoYInflationCapFloorEngine>(
                                                          marketValueEngine_);
        Handle<YoYOptionletVolatilitySurface> oldV = e->volatility();
        e->setVolatility( Handle<YoYOptionletVolatilitySurface>(
             boost::shared_ptr<YoYOptionletVolatilitySurface>(
                       new ConstantYoYOptionletVolatility(volatility,lag_))));
        capFloor_->setPricingEngine(marketValueEngine_);
        Real price = capFloor_->NPV();
        // reset the vol and the engine
        e->setVolatility(oldV);

        capFloor_->setPricingEngine(engine_);
        return price;
    }


    Real TwoMixYoYModelHelper::calibrationError() const {
        // calibrateVolatility_ = false, always so we do not
        // need to deal with that case.
        Real m = modelValue();
        Real t = marketValue();
        return std::fabs(normalizingFactor_ + t - m);
    }



    //! uses rlMakeCapFloor, one of the patches to QL 0.9.0, in rl/patches
    TwoMixSLMUPModelHelper::TwoMixSLMUPModelHelper(
                                  const Handle<Quote> &volatility,
                                  Real notional,
                                  CapFloor::Type capFloorType,
                                  const boost::shared_ptr<IborIndex>& index,
                                  Rate strike, Size n, Real normalizingFactor)
    : CalibrationHelper(volatility, index->termStructure(),
                        false), // i.e. calibrateVolatility = false
      volatility_(volatility), notional_(notional), capFloorType_(capFloorType),
      fixingDays_(index->fixingDays()),
      index_(index), strike_(strike), n_(n),
      capFloor_( MakeCapFloor(capFloorType, n*index->tenor(),
                              index, strike)
                 .withNominal(notional)
                 .asOptionlet() ),
      marketVol_(boost::shared_ptr<ConstantOptionletVolatility>(
                   new ConstantOptionletVolatility(0, index->fixingCalendar(),
                                                   Following,
                                                   volatility->value(),
                                                   index->dayCounter()))),
      normalizingFactor_(normalizingFactor) {

        marketValueEngine_ = boost::shared_ptr<BlackCapFloorEngine>(
                new BlackCapFloorEngine(index->termStructure(), volatility) );
        // N.B. You cannot re-set the volatility in the BlackCapFloorEngine.
        capFloor_->setPricingEngine(marketValueEngine_);
        marketValue_ = capFloor_->NPV();
    }


    Real TwoMixSLMUPModelHelper::modelValue() const {
        capFloor_->setPricingEngine(engine_);
        return capFloor_->NPV();
    }


    Real TwoMixSLMUPModelHelper::blackPrice(Volatility volatility) const {

        // set the vol and the engine to temporary values, then value
        // whilst saving the old one

        // actually need to create a new engine at this point
        // because you can't re-set the vol in a BlackCapFloorEngine

        boost::shared_ptr<BlackCapFloorEngine> tempEngine(
                 new BlackCapFloorEngine( marketValueEngine_->termStructure(),
                                          volatility ) );
        capFloor_->setPricingEngine(tempEngine);
        Real price = capFloor_->NPV();
        // reset the engine
        capFloor_->setPricingEngine(engine_);

        return price;
    }


    Real TwoMixSLMUPModelHelper::calibrationError() const {
        // calibrateVolatility_ = false, always so we do not
        // need to deal with that case.
        Real m = modelValue();
        Real t = marketValue();
        return std::fabs(normalizingFactor_ + t - m);
    }

}

