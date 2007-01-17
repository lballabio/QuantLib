/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2006 Cristina Duminuco

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

    Optionlet::Optionlet(
                  const boost::shared_ptr<FloatingRateCoupon>& underlying,
                  Rate strike)
    : FloatingRateCoupon(underlying->date(),
                         underlying->nominal(),
                         underlying->accrualStartDate(),
                         underlying->accrualEndDate(),
                         underlying->fixingDays(),
                         underlying->index(),
                         underlying->gearing(),
                         underlying->spread()),
      underlying_(underlying), strike_(strike) {
        registerWith(underlying);
    }

    DayCounter Optionlet::dayCounter() const {
        return underlying_->dayCounter();
    }

    Date Optionlet::fixingDate() const {
        return underlying_->fixingDate();
    }

    Rate Optionlet::indexFixing() const {
        return underlying_->indexFixing();
    }

    Rate Optionlet::strike() const {
        return strike_;
    }

    Rate Optionlet::effectiveStrike() const {
        Real csi = underlying_->gearing() > 0 ? 1.0 : -1.0;
        return (strike_ - underlying_->spread())/underlying_->gearing() ;        
    }

    void Optionlet::update() {
        notifyObservers();
    }

    void Optionlet::accept(AcyclicVisitor& v) {
        typedef FloatingRateCoupon super;
        Visitor<Optionlet>* v1 =
            dynamic_cast<Visitor<Optionlet>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            super::accept(v);
    }

    Time Optionlet::startTime() const {
        return dayCounter().yearFraction(Settings::instance().evaluationDate(),
                                         fixingDate());
    }

    double Optionlet::volatility() const {
        return std::sqrt(volatility_->blackVariance(fixingDate(),strike_ )/
                         startTime());
    }

    Caplet::Caplet(const boost::shared_ptr<FloatingRateCoupon>& underlying,
                   Rate cap)
    : Optionlet(underlying, cap) {}

    Floorlet::Floorlet(const boost::shared_ptr<FloatingRateCoupon>& underlying,
                       Rate floor)
    : Optionlet(underlying, floor) {}

    double Optionlet::amount() const {
        
        return rate() * nominal() * accrualPeriod();
    }

    Rate Caplet::rate() const {
        Rate effStrike = (strike_ - underlying_->spread())/underlying_->gearing() ;
        if (fixingDate() <= Settings::instance().evaluationDate()) {
            // the amount is determined
            return underlying_->gearing() * std::max(underlying_->indexFixing() - effStrike, 0.0);
        } else {
            // not yet determined, use Black model
            Rate fixing = 
                 blackFormula(Option::Call,
                              effStrike,
                              underlying_->adjustedFixing(),
                              std::sqrt(volatility_->blackVariance(fixingDate(),effStrike)));
            #if defined(QL_PATCH_MSVC6)
            return underlying_->gearing() * std::max(fixing,0.0);
            #else
            return underlying_->gearing() * fixing ;
            #endif
        }
    }

    Rate Floorlet::rate() const {
        Rate effStrike = (strike_ - underlying_->spread())/underlying_->gearing() ;
        if (fixingDate() <= Settings::instance().evaluationDate()) {
            // the amount is determined
            return underlying_->gearing() * std::max(effStrike - underlying_->indexFixing(), 0.0);
        } else {
            // not yet determined, use Black model
            Rate fixing = 
                 blackFormula(Option::Put,
                              effStrike, 
                              underlying_->adjustedFixing(),
                              std::sqrt(volatility_->blackVariance(fixingDate(),effStrike)));
            #if defined(QL_PATCH_MSVC6)
            return underlying_->gearing() * std::max(fixing,0.0);
            #else
            return underlying_->gearing() * fixing ;
            #endif
        }
    }

    void Optionlet::setCapletVolatility(const Handle<CapletVolatilityStructure>& vol) {
        if (!volatility_.empty())
            unregisterWith(volatility_);
        volatility_ = vol;
        if (!volatility_.empty())
            registerWith(volatility_);
        notifyObservers();
    }
}
