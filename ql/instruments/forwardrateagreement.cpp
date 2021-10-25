/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Allen Kuo

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license. You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/instruments/forwardrateagreement.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/event.hpp>

namespace QuantLib {

    ForwardRateAgreement::ForwardRateAgreement(
                           const Date& valueDate,
                           const Date& maturityDate,
                           Position::Type type,
                           Rate strikeForwardRate,
                           Real notionalAmount,
                           const ext::shared_ptr<IborIndex>& index,
                           const Handle<YieldTermStructure>& discountCurve,
                           bool useIndexedCoupon)
    : 
      fraType_(type), notionalAmount_(notionalAmount), index_(index),
      useIndexedCoupon_(useIndexedCoupon), dayCounter_(std::move(index->dayCounter())),
      calendar_(std::move(index->fixingCalendar())),
      businessDayConvention_(index->businessDayConvention()), settlementDays_(index->fixingDays()),
      payoff_(std::move(ext::shared_ptr<Payoff>())), valueDate_(valueDate),
      maturityDate_(maturityDate), discountCurve_(std::move(discountCurve)) {

        maturityDate_ = calendar_.adjust(maturityDate_, businessDayConvention_);

        registerWith(Settings::instance().evaluationDate());
        registerWith(discountCurve_);

        QL_REQUIRE(notionalAmount > 0.0, "notionalAmount must be positive");

        strikeForwardRate_ = InterestRate(strikeForwardRate,
                                          index->dayCounter(),
                                          Simple, Once);
        Real strike = notionalAmount_ *
                      strikeForwardRate_.compoundFactor(valueDate_,
                                                        maturityDate_);
        payoff_ = ext::shared_ptr<Payoff>(new ForwardTypePayoff(fraType_,
                                                                  strike));
        // incomeDiscountCurve_ is irrelevant to an FRA
        incomeDiscountCurve_ = discountCurve_;
        // income is irrelevant to FRA - set it to zero
        underlyingIncome_ = 0.0;
        registerWith(index_);
    }

    Date ForwardRateAgreement::settlementDate() const {
        return calendar_.advance(Settings::instance().evaluationDate(),
                                 settlementDays_, Days);
    }

    Date ForwardRateAgreement::fixingDate() const {
        return calendar_.advance(valueDate_,
                                 -static_cast<Integer>(settlementDays_), Days);
    }

    bool ForwardRateAgreement::isExpired() const {
        return detail::simple_event(valueDate_).hasOccurred(settlementDate());
    }

    Real ForwardRateAgreement::spotIncome(
                                    const Handle<YieldTermStructure>&) const {
        return 0.0;
    }

    // In theory, no need to implement this for a FRA (could directly
    // supply a forwardValue). For the sake of keeping a consistent
    // framework, we adhere to the concept of the forward contract as
    // defined in the base class, with an underlying having a
    // spotPrice (in this case, a loan or deposit with an NPV). Thus,
    // spotValue() is defined here.
    Real ForwardRateAgreement::spotValue() const {
        calculate();
        return notionalAmount_ *
               forwardRate().compoundFactor(valueDate_, maturityDate_) *
               discountCurve_->discount(maturityDate_);
    }

    InterestRate ForwardRateAgreement::forwardRate() const {
        calculate();
        return forwardRate_;
    }

    void ForwardRateAgreement::setupExpired() const {
        Instrument::setupExpired();
        calculateForwardRate();
    }

    void ForwardRateAgreement::performCalculations() const {
        calculateForwardRate();
        underlyingSpotValue_ = spotValue();
        underlyingIncome_    = 0.0;

        QL_REQUIRE(!discountCurve_.empty(), "null term structure set to Forward");

        ext::shared_ptr<ForwardTypePayoff> ftpayoff =
            ext::dynamic_pointer_cast<ForwardTypePayoff>(payoff_);
        Real fwdValue = forwardValue();
        NPV_ = (*ftpayoff)(fwdValue)*discountCurve_->discount(maturityDate_);
    }

    void ForwardRateAgreement::calculateForwardRate() const {
        if (useIndexedCoupon_)
            forwardRate_ =
                InterestRate(index_->fixing(fixingDate()), index_->dayCounter(), Simple, Once);
        else
            // par coupon approximation
            forwardRate_ =
                InterestRate((index_->forwardingTermStructure()->discount(valueDate_) /
                                  index_->forwardingTermStructure()->discount(maturityDate_) -
                              1.0) /
                                 index_->dayCounter().yearFraction(valueDate_, maturityDate_),
                             index_->dayCounter(), Simple, Once);
    }

    Real ForwardRateAgreement::forwardValue() const {
        calculate();
        return (underlyingSpotValue_ - underlyingIncome_) / discountCurve_->discount(maturityDate_);
    }

    InterestRate ForwardRateAgreement::impliedYield(Real underlyingSpotValue,
                                       Real forwardValue,
                                       Date settlementDate,
                                       Compounding comp,
                                       const DayCounter& dayCounter) {

        Time t = dayCounter.yearFraction(settlementDate, maturityDate_);
        Real compoundingFactor =
            forwardValue / (underlyingSpotValue - spotIncome(incomeDiscountCurve_));
        return InterestRate::impliedRate(compoundingFactor, dayCounter, comp, Annual, t);
    }
}
