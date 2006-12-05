/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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
#include <ql/Instruments/makevanillaswap.hpp>
#ifdef QL_DISABLE_DEPRECATED
#include <ql/Quotes/simplequote.hpp>
#else
#include <ql/quote.hpp>
#endif

namespace QuantLib {

    FuturesRateHelper::FuturesRateHelper(
                                     const Handle<Quote>& price,
                                     const Date& immDate,
                                     Integer nMonths,
                                     const Calendar& calendar,
                                     BusinessDayConvention convention,
                                     const DayCounter& dayCounter,
                                     const Handle<Quote>& convexityAdjustment)
    : RateHelper(price), convAdj_(convexityAdjustment) {
        QL_REQUIRE(!convAdj_.empty(), "no convexity adjustment given");
        QL_REQUIRE(convAdj_->value() >= 0.0,
                   "Negative (" << convAdj_->value() <<
                   ") futures convexity adjustment");
        earliestDate_ = immDate;
        latestDate_ =
            calendar.advance(earliestDate_, nMonths, Months, convention);
        yearFraction_ = dayCounter.yearFraction(earliestDate_, latestDate_);
        registerWith(convexityAdjustment);
    }

    FuturesRateHelper::FuturesRateHelper(const Handle<Quote>& price,
                                         const Date& immDate,
                                         Integer nMonths,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         const DayCounter& dayCounter,
                                         Rate convexityAdjustment)
    : RateHelper(price), convAdj_(Handle<Quote>(boost::shared_ptr<Quote>(
                                       new SimpleQuote(convexityAdjustment))))
    {
        QL_REQUIRE(convAdj_->value()>=0.0,
                   "Negative (" << convAdj_->value() <<
                   ") Futures convexity adjustment");
        earliestDate_ = immDate;
        latestDate_ =
            calendar.advance(earliestDate_, nMonths, Months, convention);
        yearFraction_ = dayCounter.yearFraction(earliestDate_, latestDate_);
    }

    FuturesRateHelper::FuturesRateHelper(Real price,
                                         const Date& immDate,
                                         Integer nMonths,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         const DayCounter& dayCounter,
                                         Rate convexityAdjustment)
    : RateHelper(price), convAdj_(Handle<Quote>(boost::shared_ptr<Quote>(
                                       new SimpleQuote(convexityAdjustment))))
    {
        QL_REQUIRE(convAdj_->value()>=0.0,
                   "Negative (" << convAdj_->value() <<
                   ") Futures convexity adjustment");
        earliestDate_ = immDate;
        latestDate_ =
            calendar.advance(earliestDate_, nMonths, Months, convention);
        yearFraction_ = dayCounter.yearFraction(earliestDate_, latestDate_);
    }

    Real FuturesRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        Rate forwardRate = (termStructure_->discount(earliestDate_) /
            termStructure_->discount(latestDate_)-1.0)/yearFraction_;
        #ifdef QL_EXTRA_SAFETY_CHECKS
        QL_ENSURE(convAdj_->value() >= 0.0,
                  "Negative (" << convAdj_->value() <<
                   ") futures convexity adjustment");
        #endif
        Rate futureRate = forwardRate + convAdj_->value();
        return 100 * (1.0 - futureRate);
    }

    DiscountFactor FuturesRateHelper::discountGuess() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        Rate futureRate = (100.0-quote_->value())/100.0;
        #ifdef QL_EXTRA_SAFETY_CHECKS
        QL_ENSURE(convAdj_->value() >= 0.0,
                  "Negative (" << convAdj_->value() <<
                   ") futures convexity adjustment");
        #endif
        Rate forwardRate = futureRate - convAdj_->value();
        // extrapolation shouldn't be needed if the input makes sense
        // but we'll play it safe
        return termStructure_->discount(earliestDate_,true) /
            (1.0+forwardRate*yearFraction_);
    }



    RelativeDateRateHelper::RelativeDateRateHelper(const Handle<Quote>& quote)
    : RateHelper(quote) {
        registerWith(Settings::instance().evaluationDate());
        evaluationDate_ = Settings::instance().evaluationDate();
    }

    RelativeDateRateHelper::RelativeDateRateHelper(Real quote)
    : RateHelper(quote) {
        registerWith(Settings::instance().evaluationDate());
        evaluationDate_ = Settings::instance().evaluationDate();
    }

    void RelativeDateRateHelper::update() {
        if (evaluationDate_ != Settings::instance().evaluationDate()) {
            evaluationDate_ = Settings::instance().evaluationDate();
            initializeDates();
        }
        RateHelper::update();
    }



    DepositRateHelper::DepositRateHelper(
                       const Handle<Quote>& rate,
                       const Period& tenor,
                       Integer settlementDays,
                       const Calendar& calendar,
                       BusinessDayConvention convention,
                       const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate), tenor_(tenor),
      settlementDays_(settlementDays), calendar_(calendar),
      convention_(convention), dayCounter_(dayCounter) {
        initializeDates();
    }

    DepositRateHelper::DepositRateHelper(
                       Rate rate,
                       const Period& tenor,
                       Integer settlementDays,
                       const Calendar& calendar,
                       BusinessDayConvention convention,
                       const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate), tenor_(tenor),
      settlementDays_(settlementDays), calendar_(calendar),
      convention_(convention), dayCounter_(dayCounter) {
        initializeDates();
    }

    Real DepositRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        return (termStructure_->discount(earliestDate_) /
                termStructure_->discount(latestDate_)-1.0) /
            yearFraction_;
    }

    DiscountFactor DepositRateHelper::discountGuess() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we'll play it safe - no extrapolation
        if (termStructure_->maxDate() < earliestDate_)
            return Null<Real>();
        else
            return termStructure_->discount(earliestDate_) /
                (1.0+quote_->value()*yearFraction_);
    }

    void DepositRateHelper::initializeDates() {
        earliestDate_ =
            calendar_.advance(evaluationDate_,settlementDays_,Days);
        latestDate_ = calendar_.advance(earliestDate_,tenor_,convention_);
        yearFraction_ = dayCounter_.yearFraction(earliestDate_,latestDate_);
    }


    FraRateHelper::FraRateHelper(const Handle<Quote>& rate,
                                 Integer monthsToStart, Integer monthsToEnd,
                                 Integer settlementDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate),
      monthsToStart_(monthsToStart), monthsToEnd_(monthsToEnd),
      settlementDays_(settlementDays),
      calendar_(calendar), convention_(convention),
      dayCounter_(dayCounter) {
        initializeDates();
    }

    FraRateHelper::FraRateHelper(Rate rate,
                                 Integer monthsToStart, Integer monthsToEnd,
                                 Integer settlementDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate),
      monthsToStart_(monthsToStart), monthsToEnd_(monthsToEnd),
      settlementDays_(settlementDays),
      calendar_(calendar), convention_(convention),
      dayCounter_(dayCounter) {
        initializeDates();
    }

    Real FraRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        return (termStructure_->discount(earliestDate_) /
                termStructure_->discount(latestDate_)-1.0) /
            yearFraction_;
    }

    DiscountFactor FraRateHelper::discountGuess() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // extrapolation shouldn't be needed if the input makes sense
        // but we'll play it safe
        return termStructure_->discount(earliestDate_,true) /
            (1.0+quote_->value()*yearFraction_);
    }

    void FraRateHelper::initializeDates() {
        Date settlement =
            calendar_.advance(evaluationDate_,settlementDays_,Days);
        earliestDate_ = calendar_.advance(
                               settlement,monthsToStart_,Months,convention_);
        latestDate_ = calendar_.advance(
                earliestDate_,monthsToEnd_-monthsToStart_,Months,convention_);
        yearFraction_ = dayCounter_.yearFraction(earliestDate_,latestDate_);
    }


    SwapRateHelper::SwapRateHelper(const Handle<Quote>& rate,
                                   const Period& tenor,
                                   Integer settlementDays,
                                   const Calendar& calendar,
                                   Frequency fixedFrequency,
                                   BusinessDayConvention fixedConvention,
                                   const DayCounter& fixedDayCount,
                                   const boost::shared_ptr<Xibor>& index)
    : RelativeDateRateHelper(rate),
      tenor_(tenor), settlementDays_(settlementDays),
      calendar_(calendar), fixedConvention_(fixedConvention),
      fixedFrequency_(fixedFrequency),
      fixedDayCount_(fixedDayCount),
      index_(index) {
        registerWith(index_);
        initializeDates();
    }

    SwapRateHelper::SwapRateHelper(Rate rate,
                                   const Period& tenor,
                                   Integer settlementDays,
                                   const Calendar& calendar,
                                   Frequency fixedFrequency,
                                   BusinessDayConvention fixedConvention,
                                   const DayCounter& fixedDayCount,
                                   const boost::shared_ptr<Xibor>& index)
    : RelativeDateRateHelper(rate),
      tenor_(tenor), settlementDays_(settlementDays),
      calendar_(calendar), fixedConvention_(fixedConvention),
      fixedFrequency_(fixedFrequency),
      fixedDayCount_(fixedDayCount),
      index_(index) {
        registerWith(index_);
        initializeDates();
    }

    namespace {
        void no_deletion(YieldTermStructure*) {}
    }

    void SwapRateHelper::initializeDates() {

        earliestDate_ = calendar_.advance(evaluationDate_,
                                          settlementDays_, Days);

        // dummy Libor index with curve/swap arguments
        boost::shared_ptr<Xibor> clonedIndex(new
            Xibor(index_->familyName(),
                  index_->tenor(),
                  index_->settlementDays(),
                  index_->currency(),
                  index_->calendar(),
                  index_->businessDayConvention(),
                  index_->endOfMonth(),
                  index_->dayCounter(),
                  termStructureHandle_));

        swap_ = MakeVanillaSwap(tenor_, clonedIndex, 0.0)
            .withEffectiveDate(earliestDate_)
            .withFixedLegDayCount(fixedDayCount_)
            .withFixedLegTenor(Period(fixedFrequency_))
            .withFixedLegConvention(fixedConvention_)
            .withFixedLegTerminationDateConvention(fixedConvention_);

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
                                              index_->tenor(),
                                              index_->businessDayConvention());
        latestDate_ = std::max(latestDate_,endValueDate);
        #endif
    }

    void SwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        termStructureHandle_.linkTo(
                         boost::shared_ptr<YieldTermStructure>(t,no_deletion),
                         false);
        RelativeDateRateHelper::setTermStructure(t);
    }

    Real SwapRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->recalculate();
        return swap_->fairRate();
    }

}
