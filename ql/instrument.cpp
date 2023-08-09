/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

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

#include <ql/instrument.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    Instrument::Instrument()
    : NPV_(Null<Real>()), errorEstimate_(Null<Real>()) {
        // this makes sense in general (if the evaluation date
        // changes, you probably want to recalculate) and can also
        // help avoid some edge cases when lazy objects only forward
        // their first notification.
        registerWith(Settings::instance().evaluationDate());
    }

    void Instrument::setPricingEngine(const ext::shared_ptr<PricingEngine>& e) {
        if (engine_ != nullptr)
            unregisterWith(engine_);
        engine_ = e;
        if (engine_ != nullptr)
            registerWith(engine_);
        // trigger (lazy) recalculation and notify observers
        update();
    }

    void Instrument::setupArguments(PricingEngine::arguments*) const {
        QL_FAIL("Instrument::setupArguments() not implemented");
    }

}
