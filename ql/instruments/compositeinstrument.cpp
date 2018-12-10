/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/instruments/compositeinstrument.hpp>

namespace QuantLib {

    void CompositeInstrument::add(
           const ext::shared_ptr<Instrument>& instrument, Real multiplier) {
        components_.push_back(std::make_pair(instrument,multiplier));
        registerWith(instrument);
        update();
        // When we ask for the NPV of an expired composite, the
        // components are not recalculated and thus wouldn't forward
        // later notifications according to the default behavior of
        // LazyObject instances.  This means that even if the
        // evaluation date changes so that the composite is no longer
        // expired, the instrument wouldn't be notified and thus it
        // wouldn't recalculate.  To avoid this, we override the
        // default behavior of the components.
        instrument->alwaysForwardNotifications();
    }

    void CompositeInstrument::subtract(
           const ext::shared_ptr<Instrument>& instrument, Real multiplier) {
        add(instrument, -multiplier);
    }

    bool CompositeInstrument::isExpired() const {
        for (const_iterator i=components_.begin(); i!=components_.end(); ++i) {
            if (!i->first->isExpired())
                return false;
        }
        return true;
    }

    void CompositeInstrument::performCalculations() const {
        NPV_ = 0.0;
        for (const_iterator i=components_.begin(); i!=components_.end(); ++i) {
            NPV_ += i->second * i->first->NPV();
        }
    }

    void CompositeInstrument::deepUpdate() {
        for (const_iterator i=components_.begin(); i!=components_.end(); ++i) {
            i->first->deepUpdate();
        }
        update();
    }

}

