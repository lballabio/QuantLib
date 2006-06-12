/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Allen Kuo

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

#include <ql/Instruments/forwardrateagreement.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Calendars/nullcalendar.hpp>

namespace QuantLib {

    ForwardRateAgreement::ForwardRateAgreement(
                           const Date& valueDate,
                           const Date& maturityDate,
                           Position::Type type,
                           Rate strikeForwardRate,
                           Real notionalAmount,
                           Integer settlementDays,
                           const DayCounter& dayCount,
                           const Calendar& calendar,
                           BusinessDayConvention businessDayConvention,
                           const Handle<YieldTermStructure>& discountCurve,
                           Compounding compounding,
                           Frequency frequency)
    : Forward(dayCount, calendar, businessDayConvention, settlementDays,
              boost::shared_ptr<Payoff>(), valueDate, maturityDate,
              discountCurve),
      fraType_(type), notionalAmount_(notionalAmount),
      compounding_(compounding), frequency_(frequency) {

        QL_REQUIRE(notionalAmount > 0.0, "notionalAmount must be positive");

        // do I adjust this ?
        // valueDate_ = calendar_.adjust(valueDate_,businessDayConvention_);

        forwardRate_ = discountCurve->forwardRate(valueDate_, maturityDate_,
                                                  dayCount_, compounding_,
                                                  frequency_);

        strikeForwardRate_ = InterestRate(strikeForwardRate, dayCount_,
                                          compounding_, frequency_);

        Real strike = notionalAmount_ *
                      strikeForwardRate_.compoundFactor(valueDate_,
                                                        maturityDate_);
        payoff_ = boost::shared_ptr<Payoff>(new ForwardTypePayoff(fraType_,
                                                                  strike));

        // incomeDiscountCurve_ is irrelevant to an FRA
        incomeDiscountCurve_ = discountCurve_;

        // income is irrelevant to FRA - set it to zero
        underlyingIncome_ = 0.0;
    }


    ForwardRateAgreement::ForwardRateAgreement(
                           const Date& valueDate,
                           Integer termMonths,
                           Position::Type type,
                           Rate strikeForwardRate,
                           Real notionalAmount,
                           Integer settlementDays,
                           const DayCounter& dayCount,
                           const Calendar& calendar,
                           BusinessDayConvention businessDayConvention,
                           const Handle<YieldTermStructure>& discountCurve,
                           Compounding compounding,
                           Frequency frequency)
    : Forward(dayCount, calendar, businessDayConvention, settlementDays,
              boost::shared_ptr<Payoff>(), valueDate,
              calendar.advance(valueDate,termMonths,Months,
                               businessDayConvention),
              discountCurve),
      fraType_(type), notionalAmount_(notionalAmount),
      compounding_(compounding), frequency_(frequency) {

        QL_REQUIRE(notionalAmount > 0.0, "notionalAmount must be positive");

        // do I adjust this ?
        // valueDate_ = calendar_.adjust(valueDate_,businessDayConvention_);

        forwardRate_ = discountCurve->forwardRate(valueDate_, maturityDate_,
                                                  dayCount_, compounding_,
                                                  frequency_);

        strikeForwardRate_ = InterestRate(strikeForwardRate, dayCount_,
                                          compounding_, frequency_);

        Real strike = notionalAmount_ *
                      strikeForwardRate_.compoundFactor(valueDate_,
                                                        maturityDate_);
        payoff_ = boost::shared_ptr<Payoff>(new ForwardTypePayoff(fraType_,
                                                                  strike));

        // incomeDiscountCurve_ is irrelevant to an FRA
        incomeDiscountCurve_ = discountCurve_;
    }

    Date ForwardRateAgreement::settlementDate() const {
        // take settlement at evaluationDate + settlementDays
        Date d = calendar_.advance(Settings::instance().evaluationDate(),
                                   settlementDays_, Days);
        return d;
    }

    bool ForwardRateAgreement::isExpired() const {
        #if QL_TODAYS_PAYMENTS
        return valueDate_ < settlementDate();
        #else
        return valueDate_ <= settlementDate();
        #endif
    }

    Real ForwardRateAgreement::spotIncome(
                                    const Handle<YieldTermStructure>&) const {
        return 0.0;
    }

    // in theory, no need to implement this for an FRA (could directly
    // supply a forwardValue).  For the sake of keeping a consistent
    // framework, we adhere to the concept of the forward contract as
    // defined in the base class, with an underlying having a
    // spotPrice (in this case, a loan or deposit with an NPV). Thus,
    // spotValue() is defined here.
    Real ForwardRateAgreement::spotValue() const {
        return notionalAmount_ *
               forwardRate().compoundFactor(valueDate_, maturityDate_) *
               discountCurve_->discount(maturityDate_);

    }

    InterestRate ForwardRateAgreement::forwardRate() const {
        return discountCurve_->forwardRate(valueDate_, maturityDate_,
                                           dayCount_, compounding_,
                                           frequency_);
    }

    void ForwardRateAgreement::performCalculations() const {

        underlyingSpotValue_ = spotValue();
        underlyingIncome_    = 0.0;

        Forward::performCalculations();
    }

}

