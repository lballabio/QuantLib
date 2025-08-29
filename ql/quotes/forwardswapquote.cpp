/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/quotes/forwardswapquote.hpp>
#include <ql/settings.hpp>
#include <utility>

namespace QuantLib {

    ForwardSwapQuote::ForwardSwapQuote(ext::shared_ptr<SwapIndex> swapIndex,
                                       Handle<Quote> spread,
                                       const Period& fwdStart)
    : swapIndex_(std::move(swapIndex)), spread_(std::move(spread)), fwdStart_(fwdStart) {
        registerWith(swapIndex_);
        registerWith(spread_);
        registerWith(Settings::instance().evaluationDate());
        evaluationDate_ = Settings::instance().evaluationDate();
        initializeDates();
    }

    void ForwardSwapQuote::initializeDates() {
        valueDate_ = swapIndex_->fixingCalendar().advance(
                                                evaluationDate_,
                                                swapIndex_->fixingDays()*Days,
                                                Following);
        startDate_ = swapIndex_->fixingCalendar().advance(valueDate_,
                                                          fwdStart_,
                                                          Following);
        fixingDate_ = swapIndex_->fixingDate(startDate_);
        swap_ = swapIndex_->underlyingSwap(fixingDate_);
    }

    void ForwardSwapQuote::update() {
        if (evaluationDate_ != Settings::instance().evaluationDate()) {
            evaluationDate_ = Settings::instance().evaluationDate();
            initializeDates();
        }
        LazyObject::update();
    }

    const Date& ForwardSwapQuote::valueDate() const {
        calculate();
        return valueDate_;
    }

    const Date& ForwardSwapQuote::startDate() const {
        calculate();
        return startDate_;
    }

    const Date& ForwardSwapQuote::fixingDate() const {
        calculate();
        return fixingDate_;
    }

    Real ForwardSwapQuote::value() const {
        calculate();
        return result_;
    }

    bool ForwardSwapQuote::isValid() const {
        bool swapIndexIsValid = true;
        try {
            swap_->recalculate();
        } catch (...) {
            swapIndexIsValid = false;
        }
        bool spreadIsValid = spread_.empty() ? true : spread_->isValid();
        return swapIndexIsValid && spreadIsValid;
    }

    void ForwardSwapQuote::performCalculations() const {
        // we didn't register as observers - force calculation
        swap_->recalculate();
        // weak implementation... to be improved
        static const Spread basisPoint = 1.0e-4;
        Real floatingLegNPV = swap_->floatingLegNPV();
        Spread spread = spread_.empty() ? 0.0 : spread_->value();
        Real spreadNPV = swap_->floatingLegBPS()/basisPoint*spread;
        Real totNPV = - (floatingLegNPV+spreadNPV);
        result_ = totNPV/(swap_->fixedLegBPS()/basisPoint);
    }
}
