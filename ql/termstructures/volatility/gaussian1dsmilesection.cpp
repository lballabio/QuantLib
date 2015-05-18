/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include <ql/termstructures/volatility/gaussian1dsmilesection.hpp>

#include <boost/make_shared.hpp>
#include <ql/instruments/makeswaption.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/pricingengines/blackformula.hpp>

using std::fabs;
using std::sqrt;

namespace QuantLib {

Gaussian1dSmileSection::Gaussian1dSmileSection(
    const Date &fixingDate, const boost::shared_ptr<SwapIndex> &swapIndex,
    const boost::shared_ptr<Gaussian1dModel> &model,
    const DayCounter &dc,
    const boost::shared_ptr<Gaussian1dSwaptionEngine> swaptionEngine)
    : SmileSection(fixingDate, dc, model->termStructure()->referenceDate()),
      fixingDate_(fixingDate), swapIndex_(swapIndex),
      iborIndex_(boost::shared_ptr<IborIndex>()), model_(model),
      engine_(swaptionEngine) {

    atm_ = model_->swapRate(fixingDate_, swapIndex_->tenor(), Null<Date>(), 0.0,
                            swapIndex_);
    annuity_ = model_->swapAnnuity(fixingDate_, swapIndex_->tenor(),
                                   Null<Date>(), 0.0, swapIndex_);

    if (engine_ == NULL) {
        engine_ = boost::make_shared<Gaussian1dSwaptionEngine>(
            model_, 64, 7.0, true, false,
            swapIndex_->discountingTermStructure());
    }
}

Gaussian1dSmileSection::Gaussian1dSmileSection(
    const Date &fixingDate, const boost::shared_ptr<IborIndex> &iborIndex,
    const boost::shared_ptr<Gaussian1dModel> &model,
    const DayCounter &dc,
    const boost::shared_ptr<Gaussian1dCapFloorEngine> capEngine)
    : SmileSection(fixingDate, dc, model->termStructure()->referenceDate()),
      fixingDate_(fixingDate), swapIndex_(boost::shared_ptr<SwapIndex>()),
      iborIndex_(iborIndex), model_(model), engine_(capEngine) {

    atm_ = model_->forwardRate(fixingDate_, Null<Date>(), 0.0, iborIndex_);
    CapFloor c = MakeCapFloor(CapFloor::Cap, iborIndex_->tenor(), iborIndex_,
                              Null<Real>(),
                              0 * Days).withEffectiveDate(fixingDate_, false);
    annuity_ =
        iborIndex_->dayCounter().yearFraction(c.startDate(), c.maturityDate()) *
        model_->zerobond(c.maturityDate());

    if (engine_ == NULL) {
        engine_ = boost::make_shared<Gaussian1dCapFloorEngine>(
            model_, 64, 7.0, true,
            false); // use model curve as discounting curve
    }
}

Real Gaussian1dSmileSection::atmLevel() const { return atm_; }

Real Gaussian1dSmileSection::optionPrice(Rate strike, Option::Type type,
                                         Real discount) const {

    if (swapIndex_ != NULL) {
        Swaption s = MakeSwaption(swapIndex_, fixingDate_, strike)
                         .withUnderlyingType(type == Option::Call
                                                 ? VanillaSwap::Payer
                                                 : VanillaSwap::Receiver)
                         .withPricingEngine(engine_);
        Real tmp = s.NPV();
        return tmp / annuity_ * discount;
    } else {
        CapFloor c =
            MakeCapFloor(type == Option::Call ? CapFloor::Cap : CapFloor::Floor,
                         iborIndex_->tenor(), iborIndex_, strike, 0 * Days)
                .withEffectiveDate(fixingDate_, false)
                .withPricingEngine(engine_);
        Real tmp = c.NPV();
        return tmp / annuity_ * discount;
    }
}

Real Gaussian1dSmileSection::volatilityImpl(Rate strike) const {
    Real vol = 0.0;
    try {
        Option::Type type = strike >= atm_ ? Option::Call : Option::Put;
        Real o = optionPrice(strike, type);
        vol = blackFormulaImpliedStdDev(type, strike, atm_, o) /
              sqrt(exerciseTime());
    } catch (...) {
    }
    return vol;
}
}
