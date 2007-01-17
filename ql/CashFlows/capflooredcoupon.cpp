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


#include <ql/CashFlows/capflooredcoupon.hpp>
#include <ql/PricingEngines/core.hpp>

namespace QuantLib {

    CappedFlooredCoupon::CappedFlooredCoupon(
                  const boost::shared_ptr<FloatingRateCoupon>& underlying,
                  Rate cap, Rate floor)
    : FloatingRateCoupon(underlying->date(),
                         underlying->nominal(),
                         underlying->accrualStartDate(),
                         underlying->accrualEndDate(),
                         underlying->fixingDays(),
                         underlying->index(),
                         underlying->gearing(),
                         underlying->spread()),
      underlying_(underlying) {

          if (underlying->gearing() > 0) {
            if (cap != Null<Rate>())
                cap_ = boost::shared_ptr<Optionlet>(new Caplet(underlying, cap));
            if (floor != Null<Rate>())
                floor_ = boost::shared_ptr<Optionlet>(new Floorlet(underlying, floor));
          } else {
            if (cap != Null<Rate>())
                floor_ = boost::shared_ptr<Optionlet>(new Floorlet(underlying, cap));          
            if (floor != Null<Rate>())
                cap_ = boost::shared_ptr<Optionlet>(new Caplet(underlying, floor));
          }
        registerWith(underlying);
    }

    double CappedFlooredCoupon::amount() const {
        double result = underlying_->amount();
        if (cap_)
            result -= cap_->amount();
        if (floor_)
            result += floor_->amount();
        return result;
    }

    DayCounter CappedFlooredCoupon::dayCounter() const {
        return underlying_->dayCounter();
    }

    Rate CappedFlooredCoupon::rate() const {
        QL_REQUIRE(nominal() != 0.0, "null nominal");
        QL_REQUIRE(accrualPeriod() != 0.0, "null accrual period");
        return amount()/(nominal()*accrualPeriod());
    }

    Date CappedFlooredCoupon::fixingDate() const {
        return underlying_->fixingDate();
    }

    Rate CappedFlooredCoupon::indexFixing() const {
        return underlying_->indexFixing();
    }

    Rate CappedFlooredCoupon::cap() const {
        if (underlying_->gearing() > 0) {
            if(cap_) 
                return cap_->strike();
            else
                return Rate(1.);
        } else {
             if(floor_) 
                return floor_->strike();
            else
                return Rate(1.);     
        }
    } 

    Rate CappedFlooredCoupon::floor() const {
        if (underlying_->gearing() > 0) {
            if(floor_) 
                return floor_->strike();
            else
                return Rate(0.);
        } else {
            if(cap_) 
                return cap_->strike();
            else
                return Rate(0.);        
        }
    }
    
    Rate CappedFlooredCoupon::effectiveCap() const {
        if(cap_)
            return cap_->effectiveStrike();
        else
            return Rate(1.);
    } 

    Rate CappedFlooredCoupon::effectiveFloor() const {
        if(floor_) 
            return floor_->effectiveStrike();
        else
            return Rate(0.);
    }

    Rate CappedFlooredCoupon::convexityAdjustment() const {
        return underlying_->convexityAdjustment();
    }

    void CappedFlooredCoupon::update() {
        notifyObservers();
    }

    void CappedFlooredCoupon::accept(AcyclicVisitor& v) {
        typedef FloatingRateCoupon super;
        Visitor<CappedFlooredCoupon>* v1 =
            dynamic_cast<Visitor<CappedFlooredCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            super::accept(v);
    }

    void CappedFlooredCoupon::setCapletVolatility(
               const Handle<CapletVolatilityStructure>& vol) {
        if (!volatility_.empty())
            unregisterWith(volatility_);
        volatility_ = vol;
        if (!volatility_.empty())
            registerWith(volatility_);
        notifyObservers();
        if(cap_)
            cap_->setCapletVolatility(vol);
         if(floor_)
            floor_->setCapletVolatility(vol);      
    }
}
