/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004 StatPro Italia srl
 Copyright (C) 2003 Nicolas Di Césaré

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

#include <ql/CashFlows/floatingratecoupon.hpp>

namespace QuantLib {

    FloatingRateCoupon::FloatingRateCoupon(
                         const Date& paymentDate, const Real nominal,
                         const Date& startDate, const Date& endDate,
                         const Integer fixingDays,
                         const boost::shared_ptr<InterestRateIndex>& index,
                         const Real gearing, const Spread spread,
                         const Date& refPeriodStart, const Date& refPeriodEnd,
                         const DayCounter& dayCounter,
                         bool isInArrears)
    : Coupon(nominal, paymentDate,
             startDate, endDate, refPeriodStart, refPeriodEnd),
      index_(index), dayCounter_(dayCounter),
      fixingDays_(fixingDays==Null<Integer>() ? index->fixingDays() : fixingDays),
      gearing_(gearing), spread_(spread), isInArrears_(isInArrears)
    {
        if (dayCounter_.empty())
            dayCounter_ = index_->dayCounter();
        registerWith(index_);
        registerWith(Settings::instance().evaluationDate());
    }

    Rate FloatingRateCoupon::rate() const {
        return gearing() * adjustedFixing() + spread();
    }

    Real FloatingRateCoupon::amount() const {
        return rate() * accrualPeriod() * nominal();
    }

    Real FloatingRateCoupon::accruedAmount(const Date& d) const {
        if (d <= accrualStartDate_ || d > paymentDate_) {
            return 0.0;
        } else {
            return nominal() * rate() *
                dayCounter().yearFraction(accrualStartDate_,
                                          std::min(d,accrualEndDate_),
                                          refPeriodStart_,
                                          refPeriodEnd_);
        }
    }

    DayCounter FloatingRateCoupon::dayCounter() const {
        return dayCounter_;
    }

    const boost::shared_ptr<InterestRateIndex>&
    FloatingRateCoupon::index() const {
        return index_;
    }

    Integer FloatingRateCoupon::fixingDays() const {
        return fixingDays_;
    }

    Date FloatingRateCoupon::fixingDate() const {
        // if isInArrears_ fix at the end of period
        Date refDate = isInArrears_ ? accrualEndDate_ : accrualStartDate_;
        return index_->calendar().advance(refDate, -fixingDays_*Days,
                                          Preceding);
    }

    Real FloatingRateCoupon::gearing() const {
        return gearing_;
    }

    Rate FloatingRateCoupon::indexFixing() const {
        return index_->fixing(fixingDate());
    }

    Rate FloatingRateCoupon::convexityAdjustment() const {
        return convexityAdjustmentImpl(indexFixing());
    }

    Rate FloatingRateCoupon::adjustedFixing() const {
        Rate f = indexFixing();
        return f + convexityAdjustmentImpl(f);
    }

    Spread FloatingRateCoupon::spread() const {
        return spread_;
    }

    void FloatingRateCoupon::update() {
        notifyObservers();
    }

    Rate FloatingRateCoupon::convexityAdjustmentImpl(Rate f0) const {
        if (!isInArrears_) {
            return 0.0;
        } else {
            QL_REQUIRE(!capletVolatility_.empty(), "volatility not set");
            // see Hull, 4th ed., page 550
            Date d1 = fixingDate(),
                 referenceDate = capletVolatility_->referenceDate();
            if (d1 <= referenceDate) {
                return 0.0;
            } else {
                Date d2 = index_->maturityDate(d1);
                Time tau = index_->dayCounter().yearFraction(d1, d2);
                Real variance = capletVolatility_->blackVariance(d1, f0);
                return f0*f0*variance*tau/(1.0+f0*tau);
            }
        }
    }

    void FloatingRateCoupon::accept(AcyclicVisitor& v) {
        Visitor<FloatingRateCoupon>* v1 =
            dynamic_cast<Visitor<FloatingRateCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Coupon::accept(v);
    }

    void FloatingRateCoupon::setCapletVolatility(
                                 const Handle<CapletVolatilityStructure>& v) {
        unregisterWith(capletVolatility_);
        capletVolatility_ = v;
        registerWith(capletVolatility_);
        notifyObservers();
    }
}

