/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

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

#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/imm.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/currency.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    namespace {
        void no_deletion(YieldTermStructure*) {}
    }

    FuturesRateHelper::FuturesRateHelper(const Handle<Quote>& price,
                                         const Date& immDate,
                                         Size nMonths,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         const DayCounter& dayCounter,
                                         const Handle<Quote>& convexityAdjustment)
    : RateHelper(price),
      convAdj_(convexityAdjustment) {
        QL_REQUIRE(IMM::isIMMdate(immDate, false),
                   immDate << "is not a valid IMM date");
        earliestDate_ = immDate;
        latestDate_ =
            calendar.advance(earliestDate_, nMonths, Months, convention);
        yearFraction_ = dayCounter.yearFraction(earliestDate_, latestDate_);
        registerWith(convexityAdjustment);
    }

    FuturesRateHelper::FuturesRateHelper(const Handle<Quote>& price,
                                         const Date& immDate,
                                         Size nMonths,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         const DayCounter& dayCounter,
                                         Rate convexityAdjustment)
    : RateHelper(price),
      convAdj_(Handle<Quote>(boost::shared_ptr<Quote>(new
                                            SimpleQuote(convexityAdjustment))))
    {
        QL_REQUIRE(IMM::isIMMdate(immDate, false),
                   immDate << "is not a valid IMM date");
        earliestDate_ = immDate;
        latestDate_ =
            calendar.advance(earliestDate_, nMonths, Months, convention);
        yearFraction_ = dayCounter.yearFraction(earliestDate_, latestDate_);
    }

    FuturesRateHelper::FuturesRateHelper(Real price,
                                         const Date& immDate,
                                         Size nMonths,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         const DayCounter& dayCounter,
                                         Rate convexityAdjustment)
    : RateHelper(price),
      convAdj_(Handle<Quote>(boost::shared_ptr<Quote>(new
                                            SimpleQuote(convexityAdjustment))))
    {
        QL_REQUIRE(IMM::isIMMdate(immDate, false),
                   immDate << "is not a valid IMM date");
        earliestDate_ = immDate;
        latestDate_ =
            calendar.advance(earliestDate_, nMonths, Months, convention);
        yearFraction_ = dayCounter.yearFraction(earliestDate_, latestDate_);
    }

    Real FuturesRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        Rate forwardRate = (termStructure_->discount(earliestDate_) /
            termStructure_->discount(latestDate_)-1.0)/yearFraction_;
        Rate convAdj = convAdj_->value();
        QL_ENSURE(convAdj >= 0.0,
                  "Negative (" << convAdj <<
                  ") futures convexity adjustment");
        Rate futureRate = forwardRate + convAdj;
        return 100.0 * (1.0 - futureRate);
    }

    Real FuturesRateHelper::convexityAdjustment() const {
        return convAdj_->value();
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


    DepositRateHelper::DepositRateHelper(const Handle<Quote>& rate,
                                         const Period& tenor,
                                         Natural settlementDays,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         bool endOfMonth,
                                         Natural fixingDays,
                                         const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate), settlementDays_(settlementDays) {
        index_ = boost::shared_ptr<IborIndex>(new
            IborIndex("dummy", tenor, fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_));
        initializeDates();
    }

    DepositRateHelper::DepositRateHelper(Rate rate,
                                         const Period& tenor,
                                         Natural settlementDays,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         bool endOfMonth,
                                         Natural fixingDays,
                                         const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate), settlementDays_(settlementDays) {
        index_ = boost::shared_ptr<IborIndex>(new
            IborIndex("dummy", tenor, fixingDays,
                       Currency(), calendar, convention,
                       endOfMonth, dayCounter, termStructureHandle_));
        initializeDates();
    }

    Real DepositRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        return index_->fixing(fixingDate_, true);
    }

    void DepositRateHelper::setTermStructure(YieldTermStructure* t) {
        // no need to register---the index is not lazy
        termStructureHandle_.linkTo(
                         boost::shared_ptr<YieldTermStructure>(t,no_deletion),
                         false);
        RelativeDateRateHelper::setTermStructure(t);
    }

    void DepositRateHelper::initializeDates() {
        // why not using index_->fixingDays instead of settlementDays_
        earliestDate_ = index_->fixingCalendar().advance(
            evaluationDate_, settlementDays_, Days);
        latestDate_ = index_->maturityDate(earliestDate_);
        // why not using index_->fixingDate
        fixingDate_ = index_->fixingCalendar().advance(earliestDate_,
            -static_cast<Integer>(index_->fixingDays()), Days);
    }


    FraRateHelper::FraRateHelper(const Handle<Quote>& rate,
                                 Natural monthsToStart,
                                 Natural monthsToEnd,
                                 Natural settlementDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 bool endOfMonth,
                                 Natural fixingDays,
                                 const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate), monthsToStart_(monthsToStart),
      settlementDays_(settlementDays) {
        QL_REQUIRE(monthsToEnd>monthsToStart,
                   "monthsToEnd must be grater than monthsToStart");
        index_ = boost::shared_ptr<IborIndex>(new
            IborIndex("dummy",
                      (monthsToEnd-monthsToStart)*Months,
                      fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_));
        initializeDates();
    }

    FraRateHelper::FraRateHelper(Rate rate,
                                 Natural monthsToStart,
                                 Natural monthsToEnd,
                                 Natural settlementDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 bool endOfMonth,
                                 Natural fixingDays,
                                 const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate), monthsToStart_(monthsToStart),
      settlementDays_(settlementDays) {
        QL_REQUIRE(monthsToEnd>monthsToStart,
                   "monthsToEnd must be grater than monthsToStart");
        index_ = boost::shared_ptr<IborIndex>(new
            IborIndex("dummy",
                      (monthsToEnd-monthsToStart)*Months,
                      fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_));
        initializeDates();
    }

    Real FraRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        return index_->fixing(fixingDate_,true);
    }

    void FraRateHelper::setTermStructure(YieldTermStructure* t) {
        // no need to register---the index is not lazy
        termStructureHandle_.linkTo(
                         boost::shared_ptr<YieldTermStructure>(t,no_deletion),
                         false);
        RelativeDateRateHelper::setTermStructure(t);
    }

    void FraRateHelper::initializeDates() {
        // why not using index_->fixingDays instead of settlementDays_
        Date settlement = index_->fixingCalendar().advance(evaluationDate_,
                                                           settlementDays_,
                                                           Days);
        earliestDate_ = index_->fixingCalendar().advance(
                               settlement, monthsToStart_, Months,
                               index_->businessDayConvention(),
                               index_->endOfMonth());
        latestDate_ = index_->maturityDate(earliestDate_);
        // why not using index_->fixingDate
        fixingDate_ = index_->fixingCalendar().advance(earliestDate_,
            -static_cast<Integer>(index_->fixingDays()), Days);
    }


    SwapRateHelper::SwapRateHelper(const Handle<Quote>& rate,
                                   const boost::shared_ptr<SwapIndex>& swapIndex,
                                   const Handle<Quote>& spread,
                                   const Period& fwdStart)
    : RelativeDateRateHelper(rate),
      tenor_(swapIndex->tenor()),
      calendar_(swapIndex->fixingCalendar()),
      fixedConvention_(swapIndex->fixedLegConvention()),
      fixedFrequency_(swapIndex->fixedLegTenor().frequency()),
      fixedDayCount_(swapIndex->dayCounter()),
      iborIndex_(swapIndex->iborIndex()),
      spread_(spread), fwdStart_(fwdStart) {
        registerWith(iborIndex_);
        registerWith(spread_);
        initializeDates();
    }

    SwapRateHelper::SwapRateHelper(const Handle<Quote>& rate,
                                   const Period& tenor,
                                   const Calendar& calendar,
                                   Frequency fixedFrequency,
                                   BusinessDayConvention fixedConvention,
                                   const DayCounter& fixedDayCount,
                                   const boost::shared_ptr<IborIndex>& iborIndex,
                                   const Handle<Quote>& spread,
                                   const Period& fwdStart)
    : RelativeDateRateHelper(rate),
      tenor_(tenor),
      calendar_(calendar), fixedConvention_(fixedConvention),
      fixedFrequency_(fixedFrequency),
      fixedDayCount_(fixedDayCount),
      iborIndex_(iborIndex),
      spread_(spread), fwdStart_(fwdStart) {
        registerWith(iborIndex_);
        registerWith(spread_);
        initializeDates();
    }

    SwapRateHelper::SwapRateHelper(Rate rate,
                                   const Period& tenor,
                                   const Calendar& calendar,
                                   Frequency fixedFrequency,
                                   BusinessDayConvention fixedConvention,
                                   const DayCounter& fixedDayCount,
                                   const boost::shared_ptr<IborIndex>& iborIndex,
                                   const Handle<Quote>& spread,
                                   const Period& fwdStart)
    : RelativeDateRateHelper(rate),
      tenor_(tenor),
      calendar_(calendar), fixedConvention_(fixedConvention),
      fixedFrequency_(fixedFrequency),
      fixedDayCount_(fixedDayCount),
      iborIndex_(iborIndex),
      spread_(spread), fwdStart_(fwdStart) {
        registerWith(iborIndex_);
        registerWith(spread_);
        initializeDates();
    }

    void SwapRateHelper::initializeDates() {

        // dummy ibor index with curve/swap arguments
        boost::shared_ptr<IborIndex> clonedIborIndex(new
            IborIndex(iborIndex_->familyName(),
                      iborIndex_->tenor(),
                      iborIndex_->fixingDays(),
                      iborIndex_->currency(),
                      iborIndex_->fixingCalendar(),
                      iborIndex_->businessDayConvention(),
                      iborIndex_->endOfMonth(),
                      iborIndex_->dayCounter(),
                      termStructureHandle_));

        // do not pass the spread here, as it might be a Quote
        // i.e. it can dinamically change
        swap_ = MakeVanillaSwap(tenor_, clonedIborIndex, 0.0, fwdStart_)
            .withFixedLegDayCount(fixedDayCount_)
            .withFixedLegTenor(Period(fixedFrequency_))
            .withFixedLegConvention(fixedConvention_)
            .withFixedLegTerminationDateConvention(fixedConvention_);

        earliestDate_ = swap_->startDate();

        // Usually...
        latestDate_ = swap_->maturityDate();
        // ...but due to adjustments, the last floating coupon might
        // need a later date for fixing
        #ifdef QL_USE_INDEXED_COUPON
        boost::shared_ptr<FloatingRateCoupon> lastFloating =
            boost::dynamic_pointer_cast<FloatingRateCoupon>(
                                                 swap_->floatingLeg().back());
        Date fixingValueDate = calendar_.advance(lastFloating->fixingDate(),
                                                 settlementDays_,Days);
        Date endValueDate = calendar_.advance(fixingValueDate,
                                              iborIndex_->tenor(),
                                              iborIndex_->businessDayConvention(),
                                              iborIndex_->endOfMonth());
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
        // weak implementation... to be improved
        static const Spread basisPoint = 1.0e-4;
        Real floatingLegNPV = swap_->floatingLegNPV();
        Spread spread = spread_.empty() ? 0.0 : spread_->value();
        Real spreadNPV = swap_->floatingLegBPS()/basisPoint*spread;
        Real totNPV = - (floatingLegNPV+spreadNPV);
        Real result = totNPV/(swap_->fixedLegBPS()/basisPoint);
        return result;
    }

    Spread SwapRateHelper::spread() const {
        return spread_.empty() ? 0.0 : spread_->value();
    }

    boost::shared_ptr<VanillaSwap> SwapRateHelper::swap() const {
        return swap_;
    }

}
