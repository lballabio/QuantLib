/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2007, 2008 Ferdinando Ametrano
 Copyright (C) 2007 Roland Lichters

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
#include <ql/time/imm.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/quote.hpp>
#include <ql/currency.hpp>
#include <ql/indexes/swapindex.hpp>
#ifdef QL_USE_INDEXED_COUPON
    #include <ql/cashflows/floatingratecoupon.hpp>
#endif

using boost::shared_ptr;

namespace QuantLib {

    namespace {
        void no_deletion(YieldTermStructure*) {}
    }

    FuturesRateHelper::FuturesRateHelper(const Handle<Quote>& price,
                                         const Date& immDate,
                                         Size nMonths,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         bool endOfMonth,
                                         const DayCounter& dayCounter,
                                         const Handle<Quote>& convAdj)
    : RateHelper(price), convAdj_(convAdj) {
        QL_REQUIRE(IMM::isIMMdate(immDate, false),
                   immDate << " is not a valid IMM date");
        earliestDate_ = immDate;
        latestDate_ = calendar.advance(immDate, nMonths*Months, convention,
                                                                endOfMonth);
        yearFraction_ = dayCounter.yearFraction(earliestDate_, latestDate_);

        registerWith(convAdj_);
    }

    FuturesRateHelper::FuturesRateHelper(Real price,
                                         const Date& immDate,
                                         Size nMonths,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         bool endOfMonth,
                                         const DayCounter& dayCounter,
                                         Rate convAdj)
    : RateHelper(price),
      convAdj_(Handle<Quote>(shared_ptr<Quote>(new SimpleQuote(convAdj))))
    {
        QL_REQUIRE(IMM::isIMMdate(immDate, false),
                   immDate << "is not a valid IMM date");
        earliestDate_ = immDate;
        latestDate_ = calendar.advance(immDate, nMonths*Months, convention,
                                                                endOfMonth);
        yearFraction_ = dayCounter.yearFraction(earliestDate_, latestDate_);
    }

    FuturesRateHelper::FuturesRateHelper(const Handle<Quote>& price,
                                         const Date& immDate,
                                         const shared_ptr<IborIndex>& i,
                                         const Handle<Quote>& convAdj)
    : RateHelper(price), convAdj_(convAdj) {
        QL_REQUIRE(IMM::isIMMdate(immDate, false),
                   immDate << "is not a valid IMM date");
        earliestDate_ = immDate;
        const Calendar& cal = i->fixingCalendar();
        latestDate_=cal.advance(immDate,i->tenor(),i->businessDayConvention());
        yearFraction_=i->dayCounter().yearFraction(earliestDate_, latestDate_);

        registerWith(convAdj);
    }

    FuturesRateHelper::FuturesRateHelper(Real price,
                                         const Date& immDate,
                                         const shared_ptr<IborIndex>& i,
                                         Rate convAdj)
    : RateHelper(price),
      convAdj_(Handle<Quote>(shared_ptr<Quote>(new SimpleQuote(convAdj))))
    {
        QL_REQUIRE(IMM::isIMMdate(immDate, false),
                   immDate << "is not a valid IMM date");
        earliestDate_ = immDate;
        const Calendar& cal = i->fixingCalendar();
        latestDate_=cal.advance(immDate,i->tenor(),i->businessDayConvention());
        yearFraction_=i->dayCounter().yearFraction(earliestDate_, latestDate_);
    }

    Real FuturesRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        Rate forwardRate = (termStructure_->discount(earliestDate_) /
            termStructure_->discount(latestDate_)-1.0)/yearFraction_;
        Rate convAdj = convAdj_.empty() ? 0.0 : convAdj_->value();
        QL_ENSURE(convAdj >= 0.0,
                  "Negative (" << convAdj <<
                  ") futures convexity adjustment");
        Rate futureRate = forwardRate + convAdj;
        return 100.0 * (1.0 - futureRate);
    }

    Real FuturesRateHelper::convexityAdjustment() const {
        return convAdj_.empty() ? 0.0 : convAdj_->value();
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
                                         Natural fixingDays,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         bool endOfMonth,
                                         const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate) {
        iborIndex_ = shared_ptr<IborIndex>(new
            IborIndex("no-fix", // never take fixing into account
                      tenor, fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_));
        initializeDates();
    }

    DepositRateHelper::DepositRateHelper(Rate rate,
                                         const Period& tenor,
                                         Natural fixingDays,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         bool endOfMonth,
                                         const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate) {
        iborIndex_ = shared_ptr<IborIndex>(new
            IborIndex("no-fix", // never take fixing into account
                      tenor, fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_));
        initializeDates();
    }

    DepositRateHelper::DepositRateHelper(const Handle<Quote>& rate,
                                         const shared_ptr<IborIndex>& i)
    : RelativeDateRateHelper(rate) {
        iborIndex_ = shared_ptr<IborIndex>(new
            IborIndex("no-fix", // never take fixing into account
                      i->tenor(), i->fixingDays(), Currency(),
                      i->fixingCalendar(), i->businessDayConvention(),
                      i->endOfMonth(), i->dayCounter(), termStructureHandle_));
        initializeDates();
    }

    DepositRateHelper::DepositRateHelper(Rate rate,
                                         const shared_ptr<IborIndex>& i)
    : RelativeDateRateHelper(rate) {
        iborIndex_ = shared_ptr<IborIndex>(new
            IborIndex("no-fix", // never take fixing into account
                      i->tenor(), i->fixingDays(), Currency(),
                      i->fixingCalendar(), i->businessDayConvention(),
                      i->endOfMonth(), i->dayCounter(), termStructureHandle_));
        initializeDates();
    }

    Real DepositRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        return iborIndex_->fixing(fixingDate_, true);
    }

    void DepositRateHelper::setTermStructure(YieldTermStructure* t) {
        // no need to register---the index is not lazy
        termStructureHandle_.linkTo(
                         shared_ptr<YieldTermStructure>(t,no_deletion),
                         false);
        RelativeDateRateHelper::setTermStructure(t);
    }

    void DepositRateHelper::initializeDates() {
        earliestDate_ = iborIndex_->fixingCalendar().advance(
            evaluationDate_, iborIndex_->fixingDays()*Days);
        latestDate_ = iborIndex_->maturityDate(earliestDate_);
        fixingDate_ = iborIndex_->fixingDate(earliestDate_);
    }


    FraRateHelper::FraRateHelper(const Handle<Quote>& rate,
                                 Natural monthsToStart,
                                 Natural monthsToEnd,
                                 Natural fixingDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 bool endOfMonth,
                                 const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate), monthsToStart_(monthsToStart) {
        QL_REQUIRE(monthsToEnd>monthsToStart,
                   "monthsToEnd must be grater than monthsToStart");
        iborIndex_ = shared_ptr<IborIndex>(new
            IborIndex("no-fix", // never take fixing into account
                      (monthsToEnd-monthsToStart)*Months,
                      fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_));
        initializeDates();
    }

    FraRateHelper::FraRateHelper(Rate rate,
                                 Natural monthsToStart,
                                 Natural monthsToEnd,
                                 Natural fixingDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 bool endOfMonth,
                                 const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate), monthsToStart_(monthsToStart) {
        QL_REQUIRE(monthsToEnd>monthsToStart,
                   "monthsToEnd must be grater than monthsToStart");
        iborIndex_ = shared_ptr<IborIndex>(new
            IborIndex("no-fix", // never take fixing into account
                      (monthsToEnd-monthsToStart)*Months,
                      fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_));
        initializeDates();
    }

    FraRateHelper::FraRateHelper(const Handle<Quote>& rate,
                                 Natural monthsToStart,
                                 const shared_ptr<IborIndex>& i)
    : RelativeDateRateHelper(rate), monthsToStart_(monthsToStart) {
        iborIndex_ = shared_ptr<IborIndex>(new
            IborIndex("no-fix", // never take fixing into account
                      i->tenor(), i->fixingDays(), Currency(),
                      i->fixingCalendar(), i->businessDayConvention(),
                      i->endOfMonth(), i->dayCounter(), termStructureHandle_));
        initializeDates();
    }

    FraRateHelper::FraRateHelper(Rate rate,
                                 Natural monthsToStart,
                                 const shared_ptr<IborIndex>& i)
    : RelativeDateRateHelper(rate), monthsToStart_(monthsToStart) {
        iborIndex_ = shared_ptr<IborIndex>(new
            IborIndex("no-fix", // never take fixing into account
                      i->tenor(), i->fixingDays(), Currency(),
                      i->fixingCalendar(), i->businessDayConvention(),
                      i->endOfMonth(), i->dayCounter(), termStructureHandle_));
        initializeDates();
    }

    Real FraRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        return iborIndex_->fixing(fixingDate_, true);
    }

    void FraRateHelper::setTermStructure(YieldTermStructure* t) {
        // no need to register---the index is not lazy
        termStructureHandle_.linkTo(
                         shared_ptr<YieldTermStructure>(t,no_deletion),
                         false);
        RelativeDateRateHelper::setTermStructure(t);
    }

    void FraRateHelper::initializeDates() {
        Date settlement = iborIndex_->fixingCalendar().advance(
            evaluationDate_, iborIndex_->fixingDays()*Days);
        earliestDate_ = iborIndex_->fixingCalendar().advance(
                               settlement,
                               monthsToStart_*Months,
                               iborIndex_->businessDayConvention(),
                               iborIndex_->endOfMonth());
        latestDate_ = iborIndex_->maturityDate(earliestDate_);
        fixingDate_ = iborIndex_->fixingDate(earliestDate_);
    }


    SwapRateHelper::SwapRateHelper(const Handle<Quote>& rate,
                                   const shared_ptr<SwapIndex>& swapIndex,
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
                                   const shared_ptr<IborIndex>& iborIndex,
                                   const Handle<Quote>& spread,
                                   const Period& fwdStart)
    : RelativeDateRateHelper(rate),
      tenor_(tenor),
      calendar_(calendar),
      fixedConvention_(fixedConvention),
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
                                   const shared_ptr<IborIndex>& iborIndex,
                                   const Handle<Quote>& spread,
                                   const Period& fwdStart)
    : RelativeDateRateHelper(rate),
      tenor_(tenor),
      calendar_(calendar),
      fixedConvention_(fixedConvention),
      fixedFrequency_(fixedFrequency),
      fixedDayCount_(fixedDayCount),
      iborIndex_(iborIndex),
      spread_(spread), fwdStart_(fwdStart) {
        registerWith(iborIndex_);
        registerWith(spread_);
        initializeDates();
    }

    SwapRateHelper::SwapRateHelper(Rate rate,
                                   const shared_ptr<SwapIndex>& swapIndex,
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
        registerWith(iborIndex_);               // take fixing into account
        registerWith(spread_);
        initializeDates();
    }

    void SwapRateHelper::initializeDates() {

        // dummy ibor index with curve/swap arguments
        shared_ptr<IborIndex> clonedIborIndex(new
            IborIndex(iborIndex_->familyName(),  // take fixing into account
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
        shared_ptr<FloatingRateCoupon> lastFloating =
            boost::dynamic_pointer_cast<FloatingRateCoupon>(
                                                 swap_->floatingLeg().back());
        Date fixingValueDate = calendar_.advance(lastFloating->fixingDate(),
                                                 iborIndex_->fixingDays()*Days);
        Date endValueDate = calendar_.advance(fixingValueDate,
                                              iborIndex_->tenor(),
                                              iborIndex_->businessDayConvention(),
                                              iborIndex_->endOfMonth());
        latestDate_ = std::max(latestDate_, endValueDate);
        #endif
    }

    void SwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        termStructureHandle_.linkTo(
                         shared_ptr<YieldTermStructure>(t,no_deletion),
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

    shared_ptr<VanillaSwap> SwapRateHelper::swap() const {
        return swap_;
    }

    const Period& SwapRateHelper::forwardStart() const {
        return fwdStart_;
    }

    BMASwapRateHelper::BMASwapRateHelper(
                          const Handle<Quote>& liborFraction,
                          const Period& tenor,
                          Natural settlementDays,
                          const Calendar& calendar,
                          // bma leg
                          const Period& bmaPeriod,
                          BusinessDayConvention bmaConvention,
                          const DayCounter& bmaDayCount,
                          const shared_ptr<BMAIndex>& bmaIndex,
                          // libor leg
                          const shared_ptr<IborIndex>& iborIndex)
    : RelativeDateRateHelper(liborFraction),
      tenor_(tenor), settlementDays_(settlementDays),
      calendar_(calendar),
      bmaPeriod_(bmaPeriod),
      bmaConvention_(bmaConvention),
      bmaDayCount_(bmaDayCount),
      bmaIndex_(bmaIndex),
      iborIndex_(iborIndex) {
        registerWith(iborIndex_);
        registerWith(bmaIndex_);
        initializeDates();
    }

    void BMASwapRateHelper::initializeDates() {
        earliestDate_ = calendar_.advance(evaluationDate_,
                                          settlementDays_*Days,
                                          Following);

        Date maturity = earliestDate_ + tenor_;

        // dummy BMA index with curve/swap arguments
        shared_ptr<BMAIndex> clonedIndex(new BMAIndex(termStructureHandle_));

        Schedule bmaSchedule = MakeSchedule(earliestDate_,
                                            maturity,
                                            bmaPeriod_,
                                            calendar_,
                                            bmaConvention_).backwards();

        Schedule liborSchedule =
            MakeSchedule(earliestDate_,
                         maturity,
                         iborIndex_->tenor(),
                         iborIndex_->fixingCalendar(),
                         iborIndex_->businessDayConvention())
            .endOfMonth(iborIndex_->endOfMonth())
            .backwards();

        swap_ = shared_ptr<BMASwap>(new BMASwap(BMASwap::Payer, 100.0,
                                                liborSchedule,
                                                0.75, // arbitrary
                                                0.0,
                                                iborIndex_,
                                                iborIndex_->dayCounter(),
                                                bmaSchedule,
                                                clonedIndex,
                                                bmaDayCount_));
        swap_->setPricingEngine(shared_ptr<PricingEngine>(new
            DiscountingSwapEngine(iborIndex_->termStructure())));

        Date d = calendar_.adjust(swap_->maturityDate(), Following);
        Weekday w = d.weekday();
        Date nextWednesday = (w >= 4) ?
            d + (11 - w) * Days :
            d + (4 - w) * Days;
        latestDate_ = clonedIndex->valueDate(
                         clonedIndex->fixingCalendar().adjust(nextWednesday));
    }

    void BMASwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        termStructureHandle_.linkTo(
                         shared_ptr<YieldTermStructure>(t,no_deletion),
                         false);
        RelativeDateRateHelper::setTermStructure(t);
    }

    Real BMASwapRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->recalculate();
        return swap_->fairLiborFraction();
    }

}
