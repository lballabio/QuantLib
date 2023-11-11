/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2007, 2008, 2009, 2015 Ferdinando Ametrano
 Copyright (C) 2007, 2009 Roland Lichters
 Copyright (C) 2015 Maddalena Zanzi
 Copyright (C) 2015 Paolo Mazzocchi
 Copyright (C) 2018 Matthias Lungwitz

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

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/currency.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/instruments/simplifynotificationgraph.hpp>
#include <ql/optional.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/time/asx.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/imm.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

#define APPLY_DATES(dates)                               \
    {                                                    \
        earliestDate_ = dates.earliestDate_;             \
        latestDate_ = dates.latestDate_;                 \
        maturityDate_ = dates.maturityDate_;             \
        latestRelevantDate_ = dates.latestRelevantDate_; \
        pillarDate_ = dates.pillarDate_;                 \
        yearFraction_ = dates.yearFraction_;             \
    }

namespace QuantLib {
    namespace {
        struct FuturesRateHelperDates {
            Date earliestDate_, latestDate_;
            Date maturityDate_, latestRelevantDate_, pillarDate_;
            Time yearFraction_;
        };

        void CheckDate(const Date& date, const Futures::Type type) {
            switch (type) {
                case Futures::IMM:
                    QL_REQUIRE(IMM::isIMMdate(date, false), date << " is not a valid IMM date");
                    break;
                case Futures::ASX:
                    QL_REQUIRE(ASX::isASXdate(date, false), date << " is not a valid ASX date");
                    break;
                default:
                    QL_FAIL("unknown futures type (" << Integer(type) << ')');
            }
        }

        Time DetermineYearFraction(const FuturesRateHelperDates& dates,
                                   const DayCounter& dayCounter) {
            return dayCounter.yearFraction(dates.earliestDate_, dates.maturityDate_,
                                           dates.earliestDate_, dates.maturityDate_);
        }

        FuturesRateHelperDates InitializeDates(const Date& iborStartDate,
                                               const Natural lengthInMonths,
                                               const Calendar& calendar,
                                               const BusinessDayConvention convention,
                                               const bool endOfMonth,
                                               const DayCounter& dayCounter,
                                               const Futures::Type type) {
            CheckDate(iborStartDate, type);
            FuturesRateHelperDates dates;
            dates.earliestDate_ = iborStartDate;
            dates.maturityDate_ =
                calendar.advance(iborStartDate, lengthInMonths * Months, convention, endOfMonth);
            dates.yearFraction_ = DetermineYearFraction(dates, dayCounter);
            dates.pillarDate_ = dates.latestDate_ = dates.latestRelevantDate_ = dates.maturityDate_;
            return dates;
        }

        FuturesRateHelperDates InitializeDates(const Date& iborStartDate,
                                               const Date& iborEndDate,
                                               const DayCounter& dayCounter,
                                               Futures::Type type) {
            CheckDate(iborStartDate, type);

            const auto determineMaturityDate =
                [&iborStartDate, &iborEndDate](const auto nextDateCalculator) -> Date {
                Date maturityDate;
                if (iborEndDate == Date()) {
                    // advance 3 months
                    maturityDate = nextDateCalculator(iborStartDate);
                    maturityDate = nextDateCalculator(maturityDate);
                    maturityDate = nextDateCalculator(maturityDate);
                } else {
                    QL_REQUIRE(iborEndDate > iborStartDate,
                               "end date (" << iborEndDate << ") must be greater than start date ("
                                            << iborStartDate << ')');
                    maturityDate = iborEndDate;
                }
                return maturityDate;
            };

            FuturesRateHelperDates dates;
            switch (type) {
                case Futures::IMM:
                    dates.maturityDate_ = determineMaturityDate(
                        [](const Date date) -> Date { return IMM::nextDate(date, false); });
                    break;
                case Futures::ASX:
                    dates.maturityDate_ = determineMaturityDate(
                        [](const Date date) -> Date { return ASX::nextDate(date, false); });
                    break;
                default:
                    QL_FAIL("unknown futures type (" << Integer(type) << ')');
            }
            dates.earliestDate_ = iborStartDate;
            dates.yearFraction_ = DetermineYearFraction(dates, dayCounter);
            dates.pillarDate_ = dates.latestDate_ = dates.latestRelevantDate_ = dates.maturityDate_;
            return dates;
        }

        FuturesRateHelperDates InitializeDates(const Date& iborStartDate,
                                               const ext::shared_ptr<IborIndex>& i,
                                               const Futures::Type type) {
            CheckDate(iborStartDate, type);
            FuturesRateHelperDates dates;
            dates.earliestDate_ = iborStartDate;
            const Calendar& cal = i->fixingCalendar();
            dates.maturityDate_ =
                cal.advance(iborStartDate, i->tenor(), i->businessDayConvention());
            dates.yearFraction_ = DetermineYearFraction(dates, i->dayCounter());
            dates.pillarDate_ = dates.latestDate_ = dates.latestRelevantDate_ = dates.maturityDate_;
            return dates;
        }
    } // namespace

    FuturesRateHelper::FuturesRateHelper(const Handle<Quote>& price,
                                         const Date& iborStartDate,
                                         Natural lengthInMonths,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         bool endOfMonth,
                                         const DayCounter& dayCounter,
                                         Handle<Quote> convAdj,
                                         Futures::Type type)
    : RateHelper(price), convAdj_(std::move(convAdj)) {
        const FuturesRateHelperDates dates = InitializeDates(
            iborStartDate, lengthInMonths, calendar, convention, endOfMonth, dayCounter, type);
        APPLY_DATES(dates);
        registerWith(convAdj_);
    }

    FuturesRateHelper::FuturesRateHelper(Real price,
                                         const Date& iborStartDate,
                                         Natural lengthInMonths,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         bool endOfMonth,
                                         const DayCounter& dayCounter,
                                         Rate convAdj,
                                         Futures::Type type)
    : RateHelper(price), convAdj_(Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(convAdj)))) {
        const FuturesRateHelperDates dates = InitializeDates(
            iborStartDate, lengthInMonths, calendar, convention, endOfMonth, dayCounter, type);
        APPLY_DATES(dates);
    }

    FuturesRateHelper::FuturesRateHelper(const Handle<Quote>& price,
                                         const Date& iborStartDate,
                                         const Date& iborEndDate,
                                         const DayCounter& dayCounter,
                                         Handle<Quote> convAdj,
                                         Futures::Type type)
    : RateHelper(price), convAdj_(std::move(convAdj)) {
        const FuturesRateHelperDates dates =
            InitializeDates(iborStartDate, iborEndDate, dayCounter, type);
        APPLY_DATES(dates);
        registerWith(convAdj_);
    }

    FuturesRateHelper::FuturesRateHelper(Real price,
                                         const Date& iborStartDate,
                                         const Date& iborEndDate,
                                         const DayCounter& dayCounter,
                                         Rate convAdj,
                                         Futures::Type type)
    : RateHelper(price), convAdj_(Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(convAdj)))) {
        const FuturesRateHelperDates dates =
            InitializeDates(iborStartDate, iborEndDate, dayCounter, type);
        APPLY_DATES(dates);
    }

    FuturesRateHelper::FuturesRateHelper(const Handle<Quote>& price,
                                         const Date& iborStartDate,
                                         const ext::shared_ptr<IborIndex>& i,
                                         const Handle<Quote>& convAdj,
                                         Futures::Type type)
    : RateHelper(price), convAdj_(convAdj) {
        const FuturesRateHelperDates dates = InitializeDates(iborStartDate, i, type);
        APPLY_DATES(dates);
        registerWith(convAdj);
    }

    FuturesRateHelper::FuturesRateHelper(Real price,
                                         const Date& iborStartDate,
                                         const ext::shared_ptr<IborIndex>& i,
                                         Rate convAdj,
                                         Futures::Type type)
    : RateHelper(price), convAdj_(Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(convAdj)))) {
        const FuturesRateHelperDates dates = InitializeDates(iborStartDate, i, type);
        APPLY_DATES(dates);
    }

    Real FuturesRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        Rate forwardRate = (termStructure_->discount(earliestDate_) /
            termStructure_->discount(maturityDate_) - 1.0) / yearFraction_;
        Rate convAdj = convAdj_.empty() ? 0.0 : convAdj_->value();
        // Convexity, as FRA/futures adjustment, has been used in the
        // past to take into account futures margining vs FRA.
        // Therefore, there's no requirement for it to be non-negative.
        Rate futureRate = forwardRate + convAdj;
        return 100.0 * (1.0 - futureRate);
    }

    Real FuturesRateHelper::convexityAdjustment() const {
        return convAdj_.empty() ? 0.0 : convAdj_->value();
    }

    void FuturesRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<FuturesRateHelper>*>(&v);
        if (v1 != nullptr)
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
        iborIndex_ = ext::make_shared<IborIndex>("no-fix", // never take fixing into account
                      tenor, fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_);
        DepositRateHelper::initializeDates();
    }

    DepositRateHelper::DepositRateHelper(Rate rate,
                                         const Period& tenor,
                                         Natural fixingDays,
                                         const Calendar& calendar,
                                         BusinessDayConvention convention,
                                         bool endOfMonth,
                                         const DayCounter& dayCounter)
    : RelativeDateRateHelper(rate) {
        iborIndex_ = ext::make_shared<IborIndex>("no-fix", // never take fixing into account
                      tenor, fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_);
        DepositRateHelper::initializeDates();
    }

    DepositRateHelper::DepositRateHelper(const Handle<Quote>& rate,
                                         const ext::shared_ptr<IborIndex>& i)
    : RelativeDateRateHelper(rate) {
        iborIndex_ = i->clone(termStructureHandle_);
        DepositRateHelper::initializeDates();
    }

    DepositRateHelper::DepositRateHelper(Rate rate,
                                         const ext::shared_ptr<IborIndex>& i)
    : RelativeDateRateHelper(rate) {
        iborIndex_ = i->clone(termStructureHandle_);
        DepositRateHelper::initializeDates();
    }

    Real DepositRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        // the forecast fixing flag is set to true because
        // we do not want to take fixing into account
        return iborIndex_->fixing(fixingDate_, true);
    }

    void DepositRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed---the index is not lazy
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    void DepositRateHelper::initializeDates() {
        // if the evaluation date is not a business day
        // then move to the next business day
        Date referenceDate =
            iborIndex_->fixingCalendar().adjust(evaluationDate_);
        earliestDate_ = iborIndex_->valueDate(referenceDate);
        fixingDate_ = iborIndex_->fixingDate(earliestDate_);
        maturityDate_ = iborIndex_->maturityDate(earliestDate_);
        pillarDate_ = latestDate_ = latestRelevantDate_ = maturityDate_;
    }

    void DepositRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<DepositRateHelper>*>(&v);
        if (v1 != nullptr)
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
                                 const DayCounter& dayCounter,
                                 Pillar::Choice pillarChoice,
                                 Date customPillarDate,
                                 bool useIndexedCoupon)
    : RelativeDateRateHelper(rate), periodToStart_(monthsToStart*Months),
      pillarChoice_(pillarChoice), useIndexedCoupon_(useIndexedCoupon) {
        QL_REQUIRE(monthsToEnd>monthsToStart,
                   "monthsToEnd (" << monthsToEnd <<
                   ") must be grater than monthsToStart (" << monthsToStart <<
                   ")");
        // no way to take fixing into account,
        // even if we would like to for FRA over today
        iborIndex_ = ext::make_shared<IborIndex>("no-fix", // correct family name would be needed
                      (monthsToEnd-monthsToStart)*Months,
                      fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_);
        pillarDate_ = customPillarDate;
        FraRateHelper::initializeDates();
    }

    FraRateHelper::FraRateHelper(Rate rate,
                                 Natural monthsToStart,
                                 Natural monthsToEnd,
                                 Natural fixingDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 bool endOfMonth,
                                 const DayCounter& dayCounter,
                                 Pillar::Choice pillarChoice,
                                 Date customPillarDate,
                                 bool useIndexedCoupon)
    : RelativeDateRateHelper(rate), periodToStart_(monthsToStart*Months),
      pillarChoice_(pillarChoice), useIndexedCoupon_(useIndexedCoupon) {
        QL_REQUIRE(monthsToEnd>monthsToStart,
                   "monthsToEnd (" << monthsToEnd <<
                   ") must be grater than monthsToStart (" << monthsToStart <<
                   ")");
        // no way to take fixing into account,
        // even if we would like to for FRA over today
        iborIndex_ = ext::make_shared<IborIndex>("no-fix", // correct family name would be needed
                      (monthsToEnd-monthsToStart)*Months,
                      fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_);
        pillarDate_ = customPillarDate;
        FraRateHelper::initializeDates();
    }

    FraRateHelper::FraRateHelper(const Handle<Quote>& rate,
                                 Natural monthsToStart,
                                 const ext::shared_ptr<IborIndex>& i,
                                 Pillar::Choice pillarChoice,
                                 Date customPillarDate,
                                 bool useIndexedCoupon)
    : RelativeDateRateHelper(rate), periodToStart_(monthsToStart*Months),
      pillarChoice_(pillarChoice), useIndexedCoupon_(useIndexedCoupon) {
        // take fixing into account
        iborIndex_ = i->clone(termStructureHandle_);
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        iborIndex_->unregisterWith(termStructureHandle_);
        registerWith(iborIndex_);
        pillarDate_ = customPillarDate;
        FraRateHelper::initializeDates();
    }

    FraRateHelper::FraRateHelper(Rate rate,
                                 Natural monthsToStart,
                                 const ext::shared_ptr<IborIndex>& i,
                                 Pillar::Choice pillarChoice,
                                 Date customPillarDate,
                                 bool useIndexedCoupon)
    : RelativeDateRateHelper(rate), periodToStart_(monthsToStart*Months),
      pillarChoice_(pillarChoice), useIndexedCoupon_(useIndexedCoupon) {
        // take fixing into account
        iborIndex_ = i->clone(termStructureHandle_);
        // see above
        iborIndex_->unregisterWith(termStructureHandle_);
        registerWith(iborIndex_);
        pillarDate_ = customPillarDate;
        FraRateHelper::initializeDates();
    }

    FraRateHelper::FraRateHelper(const Handle<Quote>& rate,
                                 Period periodToStart,
                                 Natural lengthInMonths,
                                 Natural fixingDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 bool endOfMonth,
                                 const DayCounter& dayCounter,
                                 Pillar::Choice pillarChoice,
                                 Date customPillarDate,
                                 bool useIndexedCoupon)
    : RelativeDateRateHelper(rate), periodToStart_(periodToStart),
      pillarChoice_(pillarChoice), useIndexedCoupon_(useIndexedCoupon) {
        // no way to take fixing into account,
        // even if we would like to for FRA over today
        iborIndex_ = ext::make_shared<IborIndex>("no-fix", // correct family name would be needed
                      lengthInMonths*Months,
                      fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_);
        pillarDate_ = customPillarDate;
        FraRateHelper::initializeDates();
    }

    FraRateHelper::FraRateHelper(Rate rate,
                                 Period periodToStart,
                                 Natural lengthInMonths,
                                 Natural fixingDays,
                                 const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 bool endOfMonth,
                                 const DayCounter& dayCounter,
                                 Pillar::Choice pillarChoice,
                                 Date customPillarDate,
                                 bool useIndexedCoupon)
    : RelativeDateRateHelper(rate), periodToStart_(periodToStart),
      pillarChoice_(pillarChoice), useIndexedCoupon_(useIndexedCoupon) {
        // no way to take fixing into account,
        // even if we would like to for FRA over today
        iborIndex_ = ext::make_shared<IborIndex>("no-fix", // correct family name would be needed
                      lengthInMonths*Months,
                      fixingDays,
                      Currency(), calendar, convention,
                      endOfMonth, dayCounter, termStructureHandle_);
        pillarDate_ = customPillarDate;
        FraRateHelper::initializeDates();
    }

    FraRateHelper::FraRateHelper(const Handle<Quote>& rate,
                                 Period periodToStart,
                                 const ext::shared_ptr<IborIndex>& i,
                                 Pillar::Choice pillarChoice,
                                 Date customPillarDate,
                                 bool useIndexedCoupon)
    : RelativeDateRateHelper(rate), periodToStart_(periodToStart),
      pillarChoice_(pillarChoice), useIndexedCoupon_(useIndexedCoupon) {
        // take fixing into account
        iborIndex_ = i->clone(termStructureHandle_);
        // see above
        iborIndex_->unregisterWith(termStructureHandle_);
        registerWith(iborIndex_);
        pillarDate_ = customPillarDate;
        FraRateHelper::initializeDates();
    }

    FraRateHelper::FraRateHelper(Rate rate,
                                 Period periodToStart,
                                 const ext::shared_ptr<IborIndex>& i,
                                 Pillar::Choice pillarChoice,
                                 Date customPillarDate,
                                 bool useIndexedCoupon)
    : RelativeDateRateHelper(rate), periodToStart_(periodToStart),
      pillarChoice_(pillarChoice), useIndexedCoupon_(useIndexedCoupon) {
        // take fixing into account
        iborIndex_ = i->clone(termStructureHandle_);
        // see above
        iborIndex_->unregisterWith(termStructureHandle_);
        registerWith(iborIndex_);
        pillarDate_ = customPillarDate;
        FraRateHelper::initializeDates();
    }

    FraRateHelper::FraRateHelper(const Handle<Quote>& rate,
                                 Natural immOffsetStart,
                                 Natural immOffsetEnd,
                                 const ext::shared_ptr<IborIndex>& i,
                                 Pillar::Choice pillarChoice,
                                 Date customPillarDate,
                                 bool useIndexedCoupon)
    : RelativeDateRateHelper(rate), immOffsetStart_(immOffsetStart), immOffsetEnd_(immOffsetEnd),
      pillarChoice_(pillarChoice), useIndexedCoupon_(useIndexedCoupon) {
        // take fixing into account
        iborIndex_ = i->clone(termStructureHandle_);
        // see above
        iborIndex_->unregisterWith(termStructureHandle_);
        registerWith(iborIndex_);
        pillarDate_ = customPillarDate;
        FraRateHelper::initializeDates();
    }

    FraRateHelper::FraRateHelper(Rate rate,
                                 Natural immOffsetStart,
                                 Natural immOffsetEnd,
                                 const ext::shared_ptr<IborIndex>& i,
                                 Pillar::Choice pillarChoice,
                                 Date customPillarDate,
                                 bool useIndexedCoupon)
    : RelativeDateRateHelper(rate), immOffsetStart_(immOffsetStart), immOffsetEnd_(immOffsetEnd),
      pillarChoice_(pillarChoice), useIndexedCoupon_(useIndexedCoupon) {
        // take fixing into account
        iborIndex_ = i->clone(termStructureHandle_);
        // see above
        iborIndex_->unregisterWith(termStructureHandle_);
        registerWith(iborIndex_);
        pillarDate_ = customPillarDate;
        FraRateHelper::initializeDates();
    }

    Real FraRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        if (useIndexedCoupon_)
            return iborIndex_->fixing(fixingDate_, true);
        else
            return (termStructure_->discount(earliestDate_) /
                        termStructure_->discount(maturityDate_) -
                    1.0) /
                   spanningTime_;
    }

    void FraRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed---the index is not lazy
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    namespace {
        Date nthImmDate(const Date& asof, const Size n) {
            Date imm = asof;
            for (Size i = 0; i < n; ++i) {
                imm = IMM::nextDate(imm, true);
            }
            return imm;
        }
    }

    void FraRateHelper::initializeDates() {
        // if the evaluation date is not a business day
        // then move to the next business day
        Date referenceDate =
            iborIndex_->fixingCalendar().adjust(evaluationDate_);
        Date spotDate = iborIndex_->fixingCalendar().advance(
            referenceDate, iborIndex_->fixingDays()*Days);
        if (periodToStart_) { // NOLINT(readability-implicit-bool-conversion)
            earliestDate_ = iborIndex_->fixingCalendar().advance(
                spotDate, *periodToStart_, iborIndex_->businessDayConvention(),
                iborIndex_->endOfMonth());
            // maturity date is calculated from spot date
            maturityDate_ = iborIndex_->fixingCalendar().advance(
                spotDate, *periodToStart_ + iborIndex_->tenor(), iborIndex_->businessDayConvention(),
                iborIndex_->endOfMonth());

        } else if ((immOffsetStart_) && (immOffsetEnd_)) { // NOLINT(readability-implicit-bool-conversion)
            earliestDate_ = iborIndex_->fixingCalendar().adjust(nthImmDate(spotDate, *immOffsetStart_));
            maturityDate_ = iborIndex_->fixingCalendar().adjust(nthImmDate(spotDate, *immOffsetEnd_));
        } else {
            QL_FAIL("neither periodToStart nor immOffsetStart/End given");
        }

        if (useIndexedCoupon_)
            // latest relevant date is calculated from earliestDate_
            latestRelevantDate_ = iborIndex_->maturityDate(earliestDate_);
        else {
            latestRelevantDate_ = maturityDate_;
            spanningTime_ = iborIndex_->dayCounter().yearFraction(earliestDate_, maturityDate_);
        }

        switch (pillarChoice_) {
          case Pillar::MaturityDate:
            pillarDate_ = maturityDate_;
            break;
          case Pillar::LastRelevantDate:
            pillarDate_ = latestRelevantDate_;
            break;
          case Pillar::CustomDate:
            // pillarDate_ already assigned at construction time
            QL_REQUIRE(pillarDate_ >= earliestDate_,
                       "pillar date (" << pillarDate_ << ") must be later "
                       "than or equal to the instrument's earliest date (" <<
                       earliestDate_ << ")");
            QL_REQUIRE(pillarDate_ <= latestRelevantDate_,
                       "pillar date (" << pillarDate_ << ") must be before "
                       "or equal to the instrument's latest relevant date (" <<
                       latestRelevantDate_ << ")");
            break;
          default:
            QL_FAIL("unknown Pillar::Choice(" << Integer(pillarChoice_) << ")");
        }

        latestDate_ = pillarDate_; // backward compatibility

        fixingDate_ = iborIndex_->fixingDate(earliestDate_);
    }

    void FraRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<FraRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }


    SwapRateHelper::SwapRateHelper(const Handle<Quote>& rate,
                                   const ext::shared_ptr<SwapIndex>& swapIndex,
                                   Handle<Quote> spread,
                                   const Period& fwdStart,
                                   Handle<YieldTermStructure> discount,
                                   Pillar::Choice pillarChoice,
                                   Date customPillarDate,
                                   bool endOfMonth,
                                   const ext::optional<bool>& useIndexedCoupons)
    : RelativeDateRateHelper(rate), settlementDays_(Null<Natural>()), tenor_(swapIndex->tenor()),
      pillarChoice_(pillarChoice), calendar_(swapIndex->fixingCalendar()),
      fixedConvention_(swapIndex->fixedLegConvention()),
      fixedFrequency_(swapIndex->fixedLegTenor().frequency()),
      fixedDayCount_(swapIndex->dayCounter()), spread_(std::move(spread)), endOfMonth_(endOfMonth),
      fwdStart_(fwdStart), discountHandle_(std::move(discount)), useIndexedCoupons_(useIndexedCoupons) {
        // take fixing into account
        iborIndex_ = swapIndex->iborIndex()->clone(termStructureHandle_);
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        iborIndex_->unregisterWith(termStructureHandle_);

        registerWith(iborIndex_);
        registerWith(spread_);
        registerWith(discountHandle_);

        pillarDate_ = customPillarDate;
        SwapRateHelper::initializeDates();
    }

    SwapRateHelper::SwapRateHelper(const Handle<Quote>& rate,
                                   const Period& tenor,
                                   Calendar calendar,
                                   Frequency fixedFrequency,
                                   BusinessDayConvention fixedConvention,
                                   DayCounter fixedDayCount,
                                   const ext::shared_ptr<IborIndex>& iborIndex,
                                   Handle<Quote> spread,
                                   const Period& fwdStart,
                                   Handle<YieldTermStructure> discount,
                                   Natural settlementDays,
                                   Pillar::Choice pillarChoice,
                                   Date customPillarDate,
                                   bool endOfMonth,
                                   const ext::optional<bool>& useIndexedCoupons)
    : RelativeDateRateHelper(rate), settlementDays_(settlementDays), tenor_(tenor),
      pillarChoice_(pillarChoice), calendar_(std::move(calendar)),
      fixedConvention_(fixedConvention), fixedFrequency_(fixedFrequency),
      fixedDayCount_(std::move(fixedDayCount)), spread_(std::move(spread)), endOfMonth_(endOfMonth),
      fwdStart_(fwdStart), discountHandle_(std::move(discount)),
      useIndexedCoupons_(useIndexedCoupons) {

        // take fixing into account
        iborIndex_ = iborIndex->clone(termStructureHandle_);
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        iborIndex_->unregisterWith(termStructureHandle_);

        registerWith(iborIndex_);
        registerWith(spread_);
        registerWith(discountHandle_);

        pillarDate_ = customPillarDate;
        SwapRateHelper::initializeDates();
    }

    SwapRateHelper::SwapRateHelper(Rate rate,
                                   const ext::shared_ptr<SwapIndex>& swapIndex,
                                   Handle<Quote> spread,
                                   const Period& fwdStart,
                                   Handle<YieldTermStructure> discount,
                                   Pillar::Choice pillarChoice,
                                   Date customPillarDate,
                                   bool endOfMonth,
                                   const ext::optional<bool>& useIndexedCoupons)
    : RelativeDateRateHelper(rate), settlementDays_(Null<Natural>()), tenor_(swapIndex->tenor()),
      pillarChoice_(pillarChoice), calendar_(swapIndex->fixingCalendar()),
      fixedConvention_(swapIndex->fixedLegConvention()),
      fixedFrequency_(swapIndex->fixedLegTenor().frequency()),
      fixedDayCount_(swapIndex->dayCounter()), spread_(std::move(spread)), endOfMonth_(endOfMonth),
      fwdStart_(fwdStart), discountHandle_(std::move(discount)),
      useIndexedCoupons_(useIndexedCoupons) {
        // take fixing into account
        iborIndex_ = swapIndex->iborIndex()->clone(termStructureHandle_);
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        iborIndex_->unregisterWith(termStructureHandle_);

        registerWith(iborIndex_);
        registerWith(spread_);
        registerWith(discountHandle_);

        pillarDate_ = customPillarDate;
        SwapRateHelper::initializeDates();
    }

    SwapRateHelper::SwapRateHelper(Rate rate,
                                   const Period& tenor,
                                   Calendar calendar,
                                   Frequency fixedFrequency,
                                   BusinessDayConvention fixedConvention,
                                   DayCounter fixedDayCount,
                                   const ext::shared_ptr<IborIndex>& iborIndex,
                                   Handle<Quote> spread,
                                   const Period& fwdStart,
                                   Handle<YieldTermStructure> discount,
                                   Natural settlementDays,
                                   Pillar::Choice pillarChoice,
                                   Date customPillarDate,
                                   bool endOfMonth,
                                   const ext::optional<bool>& useIndexedCoupons)
    : RelativeDateRateHelper(rate), settlementDays_(settlementDays), tenor_(tenor),
      pillarChoice_(pillarChoice), calendar_(std::move(calendar)),
      fixedConvention_(fixedConvention), fixedFrequency_(fixedFrequency),
      fixedDayCount_(std::move(fixedDayCount)), spread_(std::move(spread)), endOfMonth_(endOfMonth),
      fwdStart_(fwdStart), discountHandle_(std::move(discount)),
      useIndexedCoupons_(useIndexedCoupons) {

        // take fixing into account
        iborIndex_ = iborIndex->clone(termStructureHandle_);
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        iborIndex_->unregisterWith(termStructureHandle_);

        registerWith(iborIndex_);
        registerWith(spread_);
        registerWith(discountHandle_);

        pillarDate_ = customPillarDate;
        SwapRateHelper::initializeDates();
    }

    void SwapRateHelper::initializeDates() {

        // 1. do not pass the spread here, as it might be a Quote
        //    i.e. it can dynamically change
        // 2. input discount curve Handle might be empty now but it could
        //    be assigned a curve later; use a RelinkableHandle here
        swap_ = MakeVanillaSwap(tenor_, iborIndex_, 0.0, fwdStart_)
            .withSettlementDays(settlementDays_)
            .withDiscountingTermStructure(discountRelinkableHandle_)
            .withFixedLegDayCount(fixedDayCount_)
            .withFixedLegTenor(Period(fixedFrequency_))
            .withFixedLegConvention(fixedConvention_)
            .withFixedLegTerminationDateConvention(fixedConvention_)
            .withFixedLegCalendar(calendar_)
            .withFixedLegEndOfMonth(endOfMonth_)
            .withFloatingLegCalendar(calendar_)
            .withFloatingLegEndOfMonth(endOfMonth_)
            .withIndexedCoupons(useIndexedCoupons_);

        simplifyNotificationGraph(*swap_, true);

        earliestDate_ = swap_->startDate();
        maturityDate_ = swap_->maturityDate();

        ext::shared_ptr<IborCoupon> lastCoupon =
            ext::dynamic_pointer_cast<IborCoupon>(swap_->floatingLeg().back());
        latestRelevantDate_ = std::max(maturityDate_, lastCoupon->fixingEndDate());

        switch (pillarChoice_) {
          case Pillar::MaturityDate:
            pillarDate_ = maturityDate_;
            break;
          case Pillar::LastRelevantDate:
            pillarDate_ = latestRelevantDate_;
            break;
          case Pillar::CustomDate:
            // pillarDate_ already assigned at construction time
            QL_REQUIRE(pillarDate_ >= earliestDate_,
                "pillar date (" << pillarDate_ << ") must be later "
                "than or equal to the instrument's earliest date (" <<
                earliestDate_ << ")");
            QL_REQUIRE(pillarDate_ <= latestRelevantDate_,
                "pillar date (" << pillarDate_ << ") must be before "
                "or equal to the instrument's latest relevant date (" <<
                latestRelevantDate_ << ")");
            break;
          default:
            QL_FAIL("unknown Pillar::Choice(" << Integer(pillarChoice_) << ")");
        }

        latestDate_ = pillarDate_; // backward compatibility

    }

    void SwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        if (discountHandle_.empty())
            discountRelinkableHandle_.linkTo(temp, observer);
        else
            discountRelinkableHandle_.linkTo(*discountHandle_, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    Real SwapRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->deepUpdate();
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
        auto* v1 = dynamic_cast<Visitor<SwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

    BMASwapRateHelper::BMASwapRateHelper(const Handle<Quote>& liborFraction,
                                         const Period& tenor,
                                         Natural settlementDays,
                                         Calendar calendar,
                                         // bma leg
                                         const Period& bmaPeriod,
                                         BusinessDayConvention bmaConvention,
                                         DayCounter bmaDayCount,
                                         ext::shared_ptr<BMAIndex> bmaIndex,
                                         // libor leg
                                         ext::shared_ptr<IborIndex> iborIndex)
    : RelativeDateRateHelper(liborFraction), tenor_(tenor), settlementDays_(settlementDays),
      calendar_(std::move(calendar)), bmaPeriod_(bmaPeriod), bmaConvention_(bmaConvention),
      bmaDayCount_(std::move(bmaDayCount)), bmaIndex_(std::move(bmaIndex)),
      iborIndex_(std::move(iborIndex)) {
        registerWith(iborIndex_);
        registerWith(bmaIndex_);
        BMASwapRateHelper::initializeDates();
    }

    void BMASwapRateHelper::initializeDates() {
        // if the evaluation date is not a business day
        // then move to the next business day
        JointCalendar jc(calendar_,
                         iborIndex_->fixingCalendar());
        Date referenceDate = jc.adjust(evaluationDate_);
        earliestDate_ =
            calendar_.advance(referenceDate, settlementDays_ * Days, Following);

        Date maturity = earliestDate_ + tenor_;

        // dummy BMA index with curve/swap arguments
        ext::shared_ptr<BMAIndex> clonedIndex(new BMAIndex(termStructureHandle_));

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

        swap_ = ext::make_shared<BMASwap>(Swap::Payer, 100.0,
                                          liborSchedule,
                                          0.75, // arbitrary
                                          0.0,
                                          iborIndex_,
                                          iborIndex_->dayCounter(),
                                          bmaSchedule,
                                          clonedIndex,
                                          bmaDayCount_);
        swap_->setPricingEngine(ext::shared_ptr<PricingEngine>(new
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
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    Real BMASwapRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->deepUpdate();
        return swap_->fairLiborFraction();
    }

    void BMASwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<BMASwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

    FxSwapRateHelper::FxSwapRateHelper(const Handle<Quote>& fwdPoint,
                                       Handle<Quote> spotFx,
                                       const Period& tenor,
                                       Natural fixingDays,
                                       Calendar calendar,
                                       BusinessDayConvention convention,
                                       bool endOfMonth,
                                       bool isFxBaseCurrencyCollateralCurrency,
                                       Handle<YieldTermStructure> coll,
                                       Calendar tradingCalendar)
    : RelativeDateRateHelper(fwdPoint), spot_(std::move(spotFx)), tenor_(tenor),
      fixingDays_(fixingDays), cal_(std::move(calendar)), conv_(convention), eom_(endOfMonth),
      isFxBaseCurrencyCollateralCurrency_(isFxBaseCurrencyCollateralCurrency),
      collHandle_(std::move(coll)), tradingCalendar_(std::move(tradingCalendar)) {
        registerWith(spot_);
        registerWith(collHandle_);

        if (tradingCalendar_.empty())
            jointCalendar_ = cal_;
        else
            jointCalendar_ = JointCalendar(tradingCalendar_, cal_,
                                           JoinHolidays);
        FxSwapRateHelper::initializeDates();
    }

    void FxSwapRateHelper::initializeDates() {
        // if the evaluation date is not a business day
        // then move to the next business day
        Date refDate = cal_.adjust(evaluationDate_);
        earliestDate_ = cal_.advance(refDate, fixingDays_*Days);

        if (!tradingCalendar_.empty()) {
            // check if fx trade can be settled in US, if not, adjust it
            earliestDate_ = jointCalendar_.adjust(earliestDate_);
            latestDate_ = jointCalendar_.advance(earliestDate_, tenor_,
                                                 conv_, eom_);
        } else {
            latestDate_ = cal_.advance(earliestDate_, tenor_, conv_, eom_);
        }
    }

    Real FxSwapRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");

        QL_REQUIRE(!collHandle_.empty(), "collateral term structure not set");

        DiscountFactor d1 = collHandle_->discount(earliestDate_);
        DiscountFactor d2 = collHandle_->discount(latestDate_);
        Real collRatio = d1 / d2;
        d1 = termStructureHandle_->discount(earliestDate_);
        d2 = termStructureHandle_->discount(latestDate_);
        Real ratio = d1 / d2;
        Real spot = spot_->value();
        if (isFxBaseCurrencyCollateralCurrency_) {
            return (ratio/collRatio-1)*spot;
        } else {
            return (collRatio/ratio-1)*spot;
        }
    }

    void FxSwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        collRelinkableHandle_.linkTo(*collHandle_, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    void FxSwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<FxSwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
