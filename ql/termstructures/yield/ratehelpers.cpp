/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2007, 2008, 2009 Ferdinando Ametrano
 Copyright (C) 2007, 2009 Roland Lichters

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
                                         Natural lengthInMonths,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         bool endOfMonth,
                                         const DayCounter& dayCounter,
                                         const Handle<Quote>& convAdj)
    : RateHelper(price), convAdj_(convAdj) {
        QL_REQUIRE(IMM::isIMMdate(immDate, false),
                   immDate << " is not a valid IMM date");
        earliestDate_ = immDate;
        latestDate_ = calendar.advance(immDate, lengthInMonths*Months,
                                       convention, endOfMonth);
        yearFraction_ = dayCounter.yearFraction(earliestDate_, latestDate_);

        registerWith(convAdj_);
    }

    FuturesRateHelper::FuturesRateHelper(Real price,
                                         const Date& immDate,
                                         Natural lengthInMonths,
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
        latestDate_ = calendar.advance(immDate, lengthInMonths*Months,
                                       convention, endOfMonth);
        yearFraction_ = dayCounter.yearFraction(earliestDate_, latestDate_);
    }

    FuturesRateHelper::FuturesRateHelper(const Handle<Quote>& price,
                                         const Date& immDate,
                                         const Date& endDate,
                                         const DayCounter& dayCounter,
                                         const Handle<Quote>& convAdj)
    : RateHelper(price), convAdj_(convAdj) {
        QL_REQUIRE(IMM::isIMMdate(immDate, false),
                   immDate << " is not a valid IMM date");
        earliestDate_ = immDate;

        if (endDate==Date()) {
            latestDate_ = IMM::nextDate(immDate, false);
            latestDate_ = IMM::nextDate(latestDate_, false);
            latestDate_ = IMM::nextDate(latestDate_, false);
        } else { 
            QL_REQUIRE(endDate>immDate,
                       "end date (" << endDate <<
                       ") must be greater than IMM start date (" <<
                       immDate << ")");
            latestDate_ = endDate;
        }

        yearFraction_ = dayCounter.yearFraction(earliestDate_, latestDate_);

        registerWith(convAdj_);
    }

    FuturesRateHelper::FuturesRateHelper(Real price,
                                         const Date& immDate,
                                         const Date& endDate,
                                         const DayCounter& dayCounter,
                                         Rate convAdj)
    : RateHelper(price),
      convAdj_(Handle<Quote>(shared_ptr<Quote>(new SimpleQuote(convAdj))))
    {
        QL_REQUIRE(IMM::isIMMdate(immDate, false),
                   immDate << "is not a valid IMM date");
        earliestDate_ = immDate;
        
        if (endDate==Date()) {
            latestDate_ = IMM::nextDate(immDate, false);
            latestDate_ = IMM::nextDate(latestDate_, false);
            latestDate_ = IMM::nextDate(latestDate_, false);
        } else { 
            QL_REQUIRE(endDate>immDate,
                       "end date (" << endDate <<
                       ") must be greater than IMM start date (" <<
                       immDate << ")");
            latestDate_ = endDate;
        }

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

    void FuturesRateHelper::accept(AcyclicVisitor& v) {
        Visitor<FuturesRateHelper>* v1 =
            dynamic_cast<Visitor<FuturesRateHelper>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            RateHelper::accept(v);
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
        // do not use clone, as we do not want to take fixing into account
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
        // do not use clone, as we do not want to take fixing into account
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

    void DepositRateHelper::accept(AcyclicVisitor& v) {
        Visitor<DepositRateHelper>* v1 =
            dynamic_cast<Visitor<DepositRateHelper>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }


    FraRateHelper::FraRateHelper(const Handle<Quote>& rate,
                                 Natural monthsToStart,
                                 Natural monthsToEnd,
                                 Natural fixingDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 bool endOfMonth,
                                 const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate), periodToStart_(monthsToStart*Months) {
        QL_REQUIRE(monthsToEnd>monthsToStart,
                   "monthsToEnd (" << monthsToEnd <<
                   ") must be grater than monthsToStart (" << monthsToStart <<
                   ")");
        // no way to take fixing into account,
        // even if we would like to for FRA over today
        iborIndex_ = shared_ptr<IborIndex>(new
            IborIndex("no-fix", // correct family name would be needed
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
    : RelativeDateRateHelper(rate),periodToStart_(monthsToStart*Months) {
        QL_REQUIRE(monthsToEnd>monthsToStart,
                   "monthsToEnd (" << monthsToEnd <<
                   ") must be grater than monthsToStart (" << monthsToStart <<
                   ")");
        // no way to take fixing into account,
        // even if we would like to for FRA over today
        iborIndex_ = shared_ptr<IborIndex>(new
            IborIndex("no-fix", // correct family name would be needed
                      (monthsToEnd-monthsToStart)*Months,
                      fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_));
        initializeDates();
    }

    FraRateHelper::FraRateHelper(const Handle<Quote>& rate,
                                 Natural monthsToStart,
                                 const shared_ptr<IborIndex>& i)
    : RelativeDateRateHelper(rate), periodToStart_(monthsToStart*Months) {
        // take fixing into account
        iborIndex_ = i->clone(termStructureHandle_);
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        iborIndex_->unregisterWith(termStructureHandle_);
        registerWith(iborIndex_);
        initializeDates();
    }

    FraRateHelper::FraRateHelper(Rate rate,
                                 Natural monthsToStart,
                                 const shared_ptr<IborIndex>& i)
    : RelativeDateRateHelper(rate), periodToStart_(monthsToStart*Months) {
        // take fixing into account
        iborIndex_ = i->clone(termStructureHandle_);
        // see above
        iborIndex_->unregisterWith(termStructureHandle_);
        registerWith(iborIndex_);
        initializeDates();
    }

    FraRateHelper::FraRateHelper(const Handle<Quote>& rate,
                                 Period periodToStart,
                                 Natural lengthInMonths,
                                 Natural fixingDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 bool endOfMonth,
                                 const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate), periodToStart_(periodToStart) {
        // no way to take fixing into account,
        // even if we would like to for FRA over today
        iborIndex_ = shared_ptr<IborIndex>(new
            IborIndex("no-fix", // correct family name would be needed
                      lengthInMonths*Months,
                      fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_));
        initializeDates();
    }

    FraRateHelper::FraRateHelper(Rate rate,
                                 Period periodToStart,
                                 Natural lengthInMonths,
                                 Natural fixingDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 bool endOfMonth,
                                 const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate),periodToStart_(periodToStart) {
        // no way to take fixing into account,
        // even if we would like to for FRA over today
        iborIndex_ = shared_ptr<IborIndex>(new
            IborIndex("no-fix", // correct family name would be needed
                      lengthInMonths*Months,
                      fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_));
        initializeDates();
    }

    FraRateHelper::FraRateHelper(const Handle<Quote>& rate,
                                 Period periodToStart,
                                 const shared_ptr<IborIndex>& i)
    : RelativeDateRateHelper(rate), periodToStart_(periodToStart) {
        // take fixing into account
        iborIndex_ = i->clone(termStructureHandle_);
        // see above
        iborIndex_->unregisterWith(termStructureHandle_);
        registerWith(iborIndex_);
        initializeDates();
    }

    FraRateHelper::FraRateHelper(Rate rate,
                                 Period periodToStart,
                                 const shared_ptr<IborIndex>& i)
    : RelativeDateRateHelper(rate), periodToStart_(periodToStart) {
        // take fixing into account
        iborIndex_ = i->clone(termStructureHandle_);
        // see above
        iborIndex_->unregisterWith(termStructureHandle_);
        registerWith(iborIndex_);
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
                               periodToStart_,
                               iborIndex_->businessDayConvention(),
                               iborIndex_->endOfMonth());
        latestDate_ = iborIndex_->maturityDate(earliestDate_);
        fixingDate_ = iborIndex_->fixingDate(earliestDate_);
    }

    void FraRateHelper::accept(AcyclicVisitor& v) {
        Visitor<FraRateHelper>* v1 =
            dynamic_cast<Visitor<FraRateHelper>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }


    SwapRateHelper::SwapRateHelper(const Handle<Quote>& rate,
                                   const shared_ptr<SwapIndex>& swapIndex,
                                   const Handle<Quote>& spread,
                                   const Period& fwdStart,
                                   const Handle<YieldTermStructure>& discount)
    : RelativeDateRateHelper(rate),
      tenor_(swapIndex->tenor()), calendar_(swapIndex->fixingCalendar()),
      fixedConvention_(swapIndex->fixedLegConvention()),
      fixedFrequency_(swapIndex->fixedLegTenor().frequency()),
      fixedDayCount_(swapIndex->dayCounter()),
      spread_(spread),
      fwdStart_(fwdStart), discountHandle_(discount) {
        // take fixing into account
        iborIndex_ = swapIndex->iborIndex()->clone(termStructureHandle_);
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        iborIndex_->unregisterWith(termStructureHandle_);

        registerWith(iborIndex_);
        registerWith(spread_);
        registerWith(discountHandle_);
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
                                   const Period& fwdStart,
                                   const Handle<YieldTermStructure>& discount)
    : RelativeDateRateHelper(rate),
      tenor_(tenor), calendar_(calendar),
      fixedConvention_(fixedConvention),
      fixedFrequency_(fixedFrequency),
      fixedDayCount_(fixedDayCount),
      spread_(spread),
      fwdStart_(fwdStart), discountHandle_(discount) {
        // take fixing into account
        iborIndex_ = iborIndex->clone(termStructureHandle_);
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        iborIndex_->unregisterWith(termStructureHandle_);

        registerWith(iborIndex_);
        registerWith(spread_);
        registerWith(discountHandle_);
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
                                   const Period& fwdStart,
                                   const Handle<YieldTermStructure>& discount)
    : RelativeDateRateHelper(rate),
      tenor_(tenor), calendar_(calendar),
      fixedConvention_(fixedConvention),
      fixedFrequency_(fixedFrequency),
      fixedDayCount_(fixedDayCount),
      spread_(spread),
      fwdStart_(fwdStart), discountHandle_(discount) {
        // take fixing into account
        iborIndex_ = iborIndex->clone(termStructureHandle_);
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        iborIndex_->unregisterWith(termStructureHandle_);

        registerWith(iborIndex_);
        registerWith(spread_);
        registerWith(discountHandle_);
        initializeDates();
    }

    SwapRateHelper::SwapRateHelper(Rate rate,
                                   const shared_ptr<SwapIndex>& swapIndex,
                                   const Handle<Quote>& spread,
                                   const Period& fwdStart,
                                   const Handle<YieldTermStructure>& discount)
    : RelativeDateRateHelper(rate),
      tenor_(swapIndex->tenor()), calendar_(swapIndex->fixingCalendar()),
      fixedConvention_(swapIndex->fixedLegConvention()),
      fixedFrequency_(swapIndex->fixedLegTenor().frequency()),
      fixedDayCount_(swapIndex->dayCounter()),
      spread_(spread),
      fwdStart_(fwdStart), discountHandle_(discount) {
        // take fixing into account
        iborIndex_ = swapIndex->iborIndex()->clone(termStructureHandle_);
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        iborIndex_->unregisterWith(termStructureHandle_);

        registerWith(iborIndex_);
        registerWith(spread_);
        registerWith(discountHandle_);
        initializeDates();
    }

    void SwapRateHelper::initializeDates() {

        // 1. do not pass the spread here, as it might be a Quote
        //    i.e. it can dinamically change
        // 2. input discount curve Handle might be empty now but it could
        //    be assigned a curve later; use a RelinkableHandle here
        swap_ = MakeVanillaSwap(tenor_, iborIndex_, 0.0, fwdStart_)
            .withDiscountingTermStructure(discountRelinkableHandle_)
            .withFixedLegDayCount(fixedDayCount_)
            .withFixedLegTenor(Period(fixedFrequency_))
            .withFixedLegConvention(fixedConvention_)
            .withFixedLegTerminationDateConvention(fixedConvention_)
            .withFixedLegCalendar(calendar_)
            .withFloatingLegCalendar(calendar_);

        earliestDate_ = swap_->startDate();

        // Usually...
        latestDate_ = swap_->maturityDate();
        // ...but due to adjustments, the last floating coupon might
        // need a later date for fixing
        #ifdef QL_USE_INDEXED_COUPON
        shared_ptr<FloatingRateCoupon> lastFloating =
            boost::dynamic_pointer_cast<FloatingRateCoupon>(
                                                 swap_->floatingLeg().back());
        Date fixingValueDate =
            iborIndex_->valueDate(lastFloating->fixingDate());
        Date endValueDate = iborIndex_->maturityDate(fixingValueDate);
        latestDate_ = std::max(latestDate_, endValueDate);
        #endif
    }

    void SwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        shared_ptr<YieldTermStructure> temp(t, no_deletion);
        termStructureHandle_.linkTo(temp, observer);

        if (discountHandle_.empty())
            discountRelinkableHandle_.linkTo(temp, observer);
        else
            discountRelinkableHandle_.linkTo(*discountHandle_, observer);

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

    void SwapRateHelper::accept(AcyclicVisitor& v) {
        Visitor<SwapRateHelper>* v1 =
            dynamic_cast<Visitor<SwapRateHelper>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            RateHelper::accept(v);
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

        Schedule bmaSchedule =
            MakeSchedule().from(earliestDate_).to(maturity)
                          .withTenor(bmaPeriod_)
                          .withCalendar(bmaIndex_->fixingCalendar())
                          .withConvention(bmaConvention_)
                          .backwards();

        Schedule liborSchedule =
            MakeSchedule().from(earliestDate_).to(maturity)
                          .withTenor(iborIndex_->tenor())
                          .withCalendar(iborIndex_->fixingCalendar())
                          .withConvention(iborIndex_->businessDayConvention())
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
            DiscountingSwapEngine(iborIndex_->forwardingTermStructure())));

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

    void BMASwapRateHelper::accept(AcyclicVisitor& v) {
        Visitor<BMASwapRateHelper>* v1 =
            dynamic_cast<Visitor<BMASwapRateHelper>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
