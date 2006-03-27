/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2006 StatPro Italia srl

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

#include <ql/TermStructures/ratehelpers.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/DayCounters/actual360.hpp>

namespace QuantLib {

    namespace {
        void no_deletion(YieldTermStructure*) {}
    }

    RateHelper::RateHelper(const Handle<Quote>& quote)
    : quote_(quote), termStructure_(0) {
        registerWith(quote_);
    }

    RateHelper::RateHelper(Real quote)
    : quote_(Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(quote)))),
      termStructure_(0) {
        registerWith(quote_);
    }

    void RateHelper::setTermStructure(YieldTermStructure* t) {
        QL_REQUIRE(t != 0, "null term structure given");
        termStructure_ = t;
    }

    Real RateHelper::quoteError() const {
        return quote_->value()-impliedQuote();
    }



    DepositRateHelper::DepositRateHelper(
                       const Handle<Quote>& rate,
                       Integer n, TimeUnit units, Integer settlementDays,
                       const Calendar& calendar,
                       BusinessDayConvention convention,
                       const DayCounter& dayCounter)
    : RateHelper(rate), n_(n), units_(units),
      settlementDays_(settlementDays), calendar_(calendar),
      convention_(convention), dayCounter_(dayCounter) {
        registerWith(Settings::instance().evaluationDate());
    }

    DepositRateHelper::DepositRateHelper(
                       Rate rate,
                       Integer n, TimeUnit units, Integer settlementDays,
                       const Calendar& calendar,
                       BusinessDayConvention convention,
                       const DayCounter& dayCounter)
    : RateHelper(rate), n_(n), units_(units),
      settlementDays_(settlementDays), calendar_(calendar),
      convention_(convention), dayCounter_(dayCounter) {
        registerWith(Settings::instance().evaluationDate());
    }

    Real DepositRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        return (termStructure_->discount(settlement_) /
                termStructure_->discount(maturity_)-1.0) /
            yearFraction_;
    }

    DiscountFactor DepositRateHelper::discountGuess() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we'll play it safe - no extrapolation
        if (termStructure_->maxDate() < settlement_)
            return Null<Real>();
        else
            return termStructure_->discount(settlement_) /
                (1.0+quote_->value()*yearFraction_);
    }

    void DepositRateHelper::setTermStructure(YieldTermStructure* t) {
        RateHelper::setTermStructure(t);
        Date today = Settings::instance().evaluationDate();
        settlement_ = calendar_.advance(today,settlementDays_,Days);
        maturity_ = calendar_.advance(settlement_,n_,units_,convention_);
        yearFraction_ = dayCounter_.yearFraction(settlement_,maturity_);
    }

    Date DepositRateHelper::latestDate() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        return maturity_;
    }



    FraRateHelper::FraRateHelper(const Handle<Quote>& rate,
                                 Integer monthsToStart, Integer monthsToEnd,
                                 Integer settlementDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 const DayCounter& dayCounter)
    : RateHelper(rate),
      monthsToStart_(monthsToStart), monthsToEnd_(monthsToEnd),
      settlementDays_(settlementDays),
      calendar_(calendar), convention_(convention),
      dayCounter_(dayCounter) {
        registerWith(Settings::instance().evaluationDate());
    }

    FraRateHelper::FraRateHelper(Rate rate,
                                 Integer monthsToStart, Integer monthsToEnd,
                                 Integer settlementDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 const DayCounter& dayCounter)
    : RateHelper(rate),
      monthsToStart_(monthsToStart), monthsToEnd_(monthsToEnd),
      settlementDays_(settlementDays),
      calendar_(calendar), convention_(convention),
      dayCounter_(dayCounter) {
        registerWith(Settings::instance().evaluationDate());
    }

    Real FraRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        return (termStructure_->discount(start_) /
                termStructure_->discount(maturity_)-1.0) /
            yearFraction_;
    }

    DiscountFactor FraRateHelper::discountGuess() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // extrapolation shouldn't be needed if the input makes sense
        // but we'll play it safe
        return termStructure_->discount(start_,true) /
            (1.0+quote_->value()*yearFraction_);
    }

    void FraRateHelper::setTermStructure(YieldTermStructure* t) {
        RateHelper::setTermStructure(t);
        Date today = Settings::instance().evaluationDate();
        settlement_ = calendar_.advance(today,settlementDays_,Days);
        start_ = calendar_.advance(
                               settlement_,monthsToStart_,Months,convention_);
        maturity_ = calendar_.advance(
                       start_,monthsToEnd_-monthsToStart_,Months,convention_);
        yearFraction_ = dayCounter_.yearFraction(start_,maturity_);
    }

    Date FraRateHelper::latestDate() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        return maturity_;
    }


    FuturesRateHelper::FuturesRateHelper(const Handle<Quote>& price,
                                         const Date& immDate,
                                         Integer nMonths,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         const DayCounter& dayCounter)
    : RateHelper(price), immDate_(immDate),
      nMonths_(nMonths),
      calendar_(calendar), convention_(convention),
      dayCounter_(dayCounter) {
        maturity_ = calendar_.advance(immDate_, nMonths_, Months, convention_);
        yearFraction_ = dayCounter_.yearFraction(immDate_, maturity_);
    }

    FuturesRateHelper::FuturesRateHelper(const Handle<Quote>& price,
                                         const Date& immDate,
                                         const Date& matDate,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         const DayCounter& dayCounter)
    : RateHelper(price), immDate_(immDate),
      calendar_(calendar), convention_(convention),
      dayCounter_(dayCounter), maturity_(matDate) {
        yearFraction_ = dayCounter_.yearFraction(immDate_, maturity_);
    }

    FuturesRateHelper::FuturesRateHelper(Real price,
                                         const Date& immDate,
                                         Integer nMonths,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         const DayCounter& dayCounter)
    : RateHelper(price), immDate_(immDate),
      nMonths_(nMonths),
      calendar_(calendar), convention_(convention),
      dayCounter_(dayCounter) {
        maturity_ = calendar_.advance(immDate_, nMonths_, Months, convention_);
        yearFraction_ = dayCounter_.yearFraction(immDate_, maturity_);
    }

    Real FuturesRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        return 100 * (1.0-(termStructure_->discount(immDate_) /
                           termStructure_->discount(maturity_)-1.0) /
                      yearFraction_);
    }

    DiscountFactor FuturesRateHelper::discountGuess() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // extrapolation shouldn't be needed if the input makes sense
        // but we'll play it safe
        return termStructure_->discount(immDate_,true) /
            (1.0+(100.0-quote_->value())/100.0*yearFraction_);
    }

    Date FuturesRateHelper::latestDate() const {
        return maturity_;
    }


    SwapRateHelper::SwapRateHelper(const Handle<Quote>& rate,
                                   Integer n, TimeUnit units,
                                   Integer settlementDays,
                                   const Calendar& calendar,
                                   Frequency fixedFrequency,
                                   BusinessDayConvention fixedConvention,
                                   const DayCounter& fixedDayCount,
                                   Frequency floatingFrequency,
                                   BusinessDayConvention floatingConvention,
                                   const DayCounter& floatingDayCount)
    : RateHelper(rate),
      n_(n), units_(units), settlementDays_(settlementDays),
      calendar_(calendar), fixedConvention_(fixedConvention),
      floatingConvention_(floatingConvention),
      fixedFrequency_(fixedFrequency),
      floatingFrequency_(floatingFrequency),
      fixedDayCount_(fixedDayCount),
      floatingDayCount_(floatingDayCount) {
        registerWith(Settings::instance().evaluationDate());
    }

    SwapRateHelper::SwapRateHelper(
                            Rate rate,
                            Integer n, TimeUnit units, Integer settlementDays,
                            const Calendar& calendar,
                            Frequency fixedFrequency,
                            BusinessDayConvention fixedConvention,
                            const DayCounter& fixedDayCount,
                            Frequency floatingFrequency,
                            BusinessDayConvention floatingConvention,
                            const DayCounter& floatingDayCount)
    : RateHelper(rate),
      n_(n), units_(units), settlementDays_(settlementDays),
      calendar_(calendar), fixedConvention_(fixedConvention),
      floatingConvention_(floatingConvention),
      fixedFrequency_(fixedFrequency),
      floatingFrequency_(floatingFrequency),
      fixedDayCount_(fixedDayCount),
      floatingDayCount_(floatingDayCount) {
        registerWith(Settings::instance().evaluationDate());
    }

    void SwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        termStructureHandle_.linkTo(
                         boost::shared_ptr<YieldTermStructure>(t,no_deletion),
                         false);
        RateHelper::setTermStructure(t);
        Date today = Settings::instance().evaluationDate();
        settlement_ = calendar_.advance(today,settlementDays_,Days);
        Date endDate = calendar_.advance(settlement_, n_, units_,
                                         floatingConvention_);
        Schedule fixedSchedule(calendar_, settlement_, endDate,
                               fixedFrequency_, fixedConvention_);
        Schedule floatSchedule(calendar_, settlement_, endDate,
                               floatingFrequency_, floatingConvention_);
        // dummy Libor index with curve/swap arguments
        Integer fixingDays = settlementDays_;
		// Currency and dayCounter argument have to be added to SwapHelper
        boost::shared_ptr<Xibor> dummyIndex(
                                     new Xibor("dummy",
                                               12/floatingFrequency_, Months,
                                               fixingDays,
                                               Currency(),
                                               calendar_,
                                               floatingConvention_,
                                               floatingDayCount_,
                                               termStructureHandle_));

        swap_ = boost::shared_ptr<VanillaSwap>(
                   new VanillaSwap(true, 100.0,
                                   fixedSchedule, 0.0, fixedDayCount_,
                                   floatSchedule, dummyIndex, fixingDays, 0.0,
                                   floatingDayCount_, termStructureHandle_));

        // Usually...
        latestDate_ = swap_->maturity();
        // ...but due to adjustments, the last floating coupon might
        // need a later date for fixing
        #ifdef QL_USE_INDEXED_COUPON
        boost::shared_ptr<FloatingRateCoupon> lastFloating =
            boost::dynamic_pointer_cast<FloatingRateCoupon>(
                                                 swap_->floatingLeg().back());
        Date fixingValueDate = calendar_.advance(lastFloating->fixingDate(),
                                                 settlementDays_,Days);
        Date endValueDate = calendar_.advance(fixingValueDate,
                                              12/floatingFrequency_,Months,
                                              floatingConvention_);
        latestDate_ = std::max(latestDate_,endValueDate);
        #endif
    }

    Date SwapRateHelper::latestDate() const {
        QL_REQUIRE(termStructure_ != 0, "null term structure set");
        return latestDate_;
    }

    Real SwapRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->recalculate();
        return swap_->fairRate();
    }

}
