/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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

#include <ql/CashFlows/capfloorlet.hpp>
#include <ql/PricingEngines/core.hpp>

namespace QuantLib {

    CapFloorlet::CapFloorlet(
                  const boost::shared_ptr<FloatingRateCoupon>& underlying,
                  Rate strike,
                  const boost::shared_ptr<BlackCapFloorModel>& model)
    : FloatingRateCoupon(underlying->date(),
                         underlying->nominal(),
                         underlying->accrualStartDate(),
                         underlying->accrualEndDate(),
                         underlying->fixingDays(),
                         underlying->index()),
      underlying_(underlying), strike_(strike), model_(model) {
        registerWith(underlying);
    }

    DayCounter CapFloorlet::dayCounter() const {
        return underlying_->dayCounter();
    }

    Date CapFloorlet::fixingDate() const {
        return underlying_->fixingDate();
    }

    Rate CapFloorlet::indexFixing() const {
        return underlying_->indexFixing();
    }

    void CapFloorlet::update() {
        notifyObservers();
    }

    void CapFloorlet::accept(AcyclicVisitor& v) {
        typedef FloatingRateCoupon super;
        Visitor<CapFloorlet>* v1 =
            dynamic_cast<Visitor<CapFloorlet>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            super::accept(v);
    }

    Time CapFloorlet::startTime() const {
        return model_->volTermStructure()->dayCounter().yearFraction(
                                    Settings::instance().evaluationDate(),
                                    fixingDate());
    }

    double CapFloorlet::volatility() const {
        return model_->volTermStructure()->volatility(
                                    Settings::instance().evaluationDate(),
                                    fixingDate(),
                                    strike_);
    }

    Caplet::Caplet(const boost::shared_ptr<FloatingRateCoupon>& underlying,
                   Rate cap,
                   const boost::shared_ptr<BlackCapFloorModel>& model)
    : CapFloorlet(underlying,cap,model) {}

    Floorlet::Floorlet(const boost::shared_ptr<FloatingRateCoupon>& underl,
                       Rate floor,
                       const boost::shared_ptr<BlackCapFloorModel>& model)
    : CapFloorlet(underl,floor,model) {}

    double CapFloorlet::amount() const {
        return rate() * nominal() * accrualPeriod();
    }

    Rate Caplet::rate() const {
        if (fixingDate() <= Settings::instance().evaluationDate()) {
            // the amount is determined
            return std::max(underlying_->rate()-strike_, 0.0);
        } else {
            // not yet determined, use Black model
            Rate fixing =
                BlackModel::formula(underlying_->rate(),
                                    strike_,
                                    volatility()*std::sqrt(startTime()),
                                    1);
            #if defined(QL_PATCH_MSVC6)
            return std::max(fixing,0.0);
            #else
            return fixing;
            #endif
        }
    }

    Rate Floorlet::rate() const {
        if (fixingDate() <= Settings::instance().evaluationDate()) {
            // the amount is determined
            return std::max(strike_-underlying_->rate(), 0.0);
        } else {
            // not yet determined, use Black model
            Rate fixing =
                BlackModel::formula(underlying_->rate(),
                                    strike_,
                                    volatility()*std::sqrt(startTime()),
                                    -1);
            #if defined(QL_PATCH_MSVC6)
            return std::max(fixing,0.0);
            #else
            return fixing;
            #endif
        }
    }

}
