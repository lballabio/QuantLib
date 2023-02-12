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

#include <ql/event.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/forwardrateagreement.hpp>
#include <utility>
#include <iostream>

namespace QuantLib {

    ForwardRateAgreement::ForwardRateAgreement(const Date& valueDate,
                                               const Date& maturityDate,
                                               Position::Type type,
                                               Rate strikeForwardRate,
                                               Real notionalAmount,
                                               const ext::shared_ptr<IborIndex>& index,
                                               Handle<YieldTermStructure> discountCurve,
                                               bool useIndexedCoupon)
    : fraType_(type), notionalAmount_(notionalAmount), index_(index),
      useIndexedCoupon_(useIndexedCoupon), dayCounter_(index->dayCounter()),
      calendar_(index->fixingCalendar()), businessDayConvention_(index->businessDayConvention()),
      valueDate_(valueDate), maturityDate_(maturityDate),
      discountCurve_(std::move(discountCurve)) {

        maturityDate_ = calendar_.adjust(maturityDate_, businessDayConvention_);

        registerWith(Settings::instance().evaluationDate());
        registerWith(discountCurve_);

        QL_REQUIRE(notionalAmount > 0.0, "notionalAmount must be positive");

        strikeForwardRate_ = InterestRate(strikeForwardRate,
                                          index->dayCounter(),
                                          Simple, Once);
        registerWith(index_);
    }

    ForwardRateAgreement::ForwardRateAgreement(const Date& valueDate,
                                               Position::Type type,
                                               Rate strikeForwardRate,
                                               Real notionalAmount,
                                               const ext::shared_ptr<IborIndex>& index,
                                               Handle<YieldTermStructure> discountCurve)
    : ForwardRateAgreement(valueDate, index->maturityDate(valueDate), type, strikeForwardRate,
                           notionalAmount, index, std::move(discountCurve), true) {}

    ForwardRateAgreement::ForwardRateAgreement(Position::Type type,
                                               const Date& valueDate,
                                               Rate strikeForwardRate,
                                               Real notionalAmount,
                                               const ext::shared_ptr<IborIndex>& index,
                                               Handle<YieldTermStructure> discountCurve,
                                               bool useIndexedCoupon)
    : fraType_(type), valueDate_(valueDate), notionalAmount_(notionalAmount),
      index_(index), discountCurve_(std::move(discountCurve)),
      useIndexedCoupon_(useIndexedCoupon), 
      dayCounter_(index->dayCounter()), calendar_(index->fixingCalendar()),
      businessDayConvention_(index->businessDayConvention()),
      maturityDate_(index->maturityDate(valueDate)) {

        QL_REQUIRE(notionalAmount > 0.0, "notionalAmount must be positive");

        registerWith(Settings::instance().evaluationDate());
        registerWith(discountCurve_);
        registerWith(index_);
        
        maturityDate_ = calendar_.adjust(maturityDate_, businessDayConvention_);
        QL_REQUIRE(valueDate_ < maturityDate_, "valueDate must be earlier than maturityDate");
        strikeForwardRate_ = InterestRate(strikeForwardRate,
                                          index->dayCounter(),
                                          Simple, Once);
    }

    ForwardRateAgreement::ForwardRateAgreement(const Date& valueDate,
                                               const Date& maturityDate,
                                               Position::Type type,
                                               Rate strikeForwardRate,
                                               Rate referenceRate,
                                               Real notionalAmount,
                                               const DayCounter dayCounter,
                                               const Calendar fixingCalendar,
                                               const BusinessDayConvention businessDayConvention,
                                               Handle<YieldTermStructure> discountCurve)
    : valueDate_(valueDate), maturityDate_(maturityDate),
      fraType_(type), notionalAmount_(notionalAmount),
      dayCounter_(dayCounter), calendar_(fixingCalendar),
      businessDayConvention_(businessDayConvention),
      discountCurve_(std::move(discountCurve)),
      useIndexedCoupon_(false) {

        QL_REQUIRE(notionalAmount > 0.0, "notionalAmount must be positive");

        registerWith(Settings::instance().evaluationDate());
        registerWith(discountCurve_);
        
        maturityDate_ = calendar_.adjust(maturityDate_, businessDayConvention_);
        QL_REQUIRE(valueDate_ < maturityDate_, "valueDate must be earlier than maturityDate");
        strikeForwardRate_ = InterestRate(strikeForwardRate,
                                          dayCounter,
                                          Simple, Once);
        referenceRate_ = InterestRate(referenceRate,
                                      dayCounter,
                                      Simple, Once);    
        valueDateTime_ = dayCounter_.yearFraction(Settings::instance().evaluationDate(), valueDate_);
        maturityDateTime_ = dayCounter_.yearFraction(Settings::instance().evaluationDate(), maturityDate_);                              
    }

    Date ForwardRateAgreement::fixingDate() const {
        if (index_)
            return index_->fixingDate(valueDate_);
        else // if FRA is constructed without the index it returns the valueDate
            return valueDate_;
    }

    bool ForwardRateAgreement::isExpired() const {
        return detail::simple_event(valueDate_).hasOccurred();
    }

    Real ForwardRateAgreement::amount() const {
        calculate();
        return amount_;
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
        calculateAmount();

        if (!discountCurve_.empty() || index_) {

            Handle<YieldTermStructure> discount =
                discountCurve_.empty() ? index_->forwardingTermStructure() : discountCurve_;

            NPV_ = amount_ * discount->discount(valueDate_);
        } else
            NPV_ = amount_ * referenceRate_.discountFactor(valueDateTime_);
    }

    void ForwardRateAgreement::calculateForwardRate() const {
        if (useIndexedCoupon_ && index_)
            forwardRate_ =
                InterestRate(index_->fixing(fixingDate()), index_->dayCounter(), Simple, Once);
        else if (!useIndexedCoupon_ && index_)
            // par coupon approximation
            forwardRate_ =
                InterestRate((index_->forwardingTermStructure()->discount(valueDate_) /
                                  index_->forwardingTermStructure()->discount(maturityDate_) -
                              1.0) /
                                 index_->dayCounter().yearFraction(valueDate_, maturityDate_),
                             index_->dayCounter(), Simple, Once);
        else
            forwardRate_ =
                InterestRate((referenceRate_.discountFactor(valueDateTime_) /
                              referenceRate_.discountFactor(maturityDateTime_) -
                              1.0) /
                              dayCounter_.yearFraction(valueDate_, maturityDate_),
                            dayCounter_, Simple, Once);          
                          
    }

    void ForwardRateAgreement::calculateAmount() const {
        calculateForwardRate();
        Integer sign = fraType_ == Position::Long? 1 : -1;

        Rate F = forwardRate_.rate();
        Rate K = strikeForwardRate_.rate();
        Time T = forwardRate_.dayCounter().yearFraction(valueDate_, maturityDate_);

        amount_ = notionalAmount_ * sign * (F - K) * T / (1.0 + F * T);
    }

}
