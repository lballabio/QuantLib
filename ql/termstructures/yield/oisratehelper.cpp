/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2012 Roland Lichters
 Copyright (C) 2009, 2012 Ferdinando Ametrano

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

#include <ql/instruments/makeois.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/yield/oisratehelper.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {

    OISRateHelper::OISRateHelper(Natural settlementDays,
                                 const Period& tenor, // swap maturity
                                 const Handle<Quote>& fixedRate,
                                 const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                 Handle<YieldTermStructure> discount,
                                 bool telescopicValueDates,
                                 Natural paymentLag,
                                 BusinessDayConvention paymentConvention,
                                 Frequency paymentFrequency,
                                 Calendar paymentCalendar,
                                 const Period& forwardStart,
                                 const Spread overnightSpread,
                                 Pillar::Choice pillar,
                                 Date customPillarDate,
                                 RateAveraging::Type averagingMethod,
                                 ext::optional<bool> endOfMonth)
    : RelativeDateRateHelper(fixedRate), pillarChoice_(pillar), settlementDays_(settlementDays), tenor_(tenor),
      discountHandle_(std::move(discount)), telescopicValueDates_(telescopicValueDates),
      paymentLag_(paymentLag), paymentConvention_(paymentConvention),
      paymentFrequency_(paymentFrequency), paymentCalendar_(std::move(paymentCalendar)),
      forwardStart_(forwardStart), overnightSpread_(overnightSpread),
      averagingMethod_(averagingMethod), endOfMonth_(endOfMonth) {

        overnightIndex_ =
            ext::dynamic_pointer_cast<OvernightIndex>(overnightIndex->clone(termStructureHandle_));
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        overnightIndex_->unregisterWith(termStructureHandle_);

        registerWith(overnightIndex_);
        registerWith(discountHandle_);

        pillarDate_ = customPillarDate;
        OISRateHelper::initializeDates();
    }

    void OISRateHelper::initializeDates() {

        // input discount curve Handle might be empty now but it could
        //    be assigned a curve later; use a RelinkableHandle here
        MakeOIS tmp = MakeOIS(tenor_, overnightIndex_, 0.0, forwardStart_)
            .withDiscountingTermStructure(discountRelinkableHandle_)
            .withSettlementDays(settlementDays_)
            .withTelescopicValueDates(telescopicValueDates_)
            .withPaymentLag(paymentLag_)
            .withPaymentAdjustment(paymentConvention_)
            .withPaymentFrequency(paymentFrequency_)
            .withPaymentCalendar(paymentCalendar_)
            .withOvernightLegSpread(overnightSpread_)
            .withAveragingMethod(averagingMethod_);
        if (endOfMonth_) {
            swap_ = tmp.withEndOfMonth(*endOfMonth_);
        } else {
            swap_ = tmp;
        }

        earliestDate_ = swap_->startDate();
        maturityDate_ = swap_->maturityDate();

        Date lastPaymentDate = std::max(swap_->overnightLeg().back()->date(),
                                        swap_->fixedLeg().back()->date());
        latestRelevantDate_ = std::max(maturityDate_, lastPaymentDate);

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

        latestDate_ = std::max(swap_->maturityDate(), lastPaymentDate);
    }

    void OISRateHelper::setTermStructure(YieldTermStructure* t) {
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

    Real OISRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->deepUpdate();
        return swap_->fairRate();
    }

    void OISRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<OISRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

    DatedOISRateHelper::DatedOISRateHelper(const Date& startDate,
                                           const Date& endDate,
                                           const Handle<Quote>& fixedRate,
                                           const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                           Handle<YieldTermStructure> discount,
                                           bool telescopicValueDates,
                                           RateAveraging::Type averagingMethod,
                                           Natural paymentLag,
                                           BusinessDayConvention paymentConvention,
                                           Frequency paymentFrequency,
                                           const Calendar& paymentCalendar,
                                           const Period& forwardStart,
                                           Spread overnightSpread,
                                           ext::optional<bool> endOfMonth)
    : RateHelper(fixedRate), discountHandle_(std::move(discount)),
      telescopicValueDates_(telescopicValueDates), averagingMethod_(averagingMethod) {

        auto clonedOvernightIndex =
            ext::dynamic_pointer_cast<OvernightIndex>(overnightIndex->clone(termStructureHandle_));
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        clonedOvernightIndex->unregisterWith(termStructureHandle_);

        registerWith(clonedOvernightIndex);
        registerWith(discountHandle_);

        // input discount curve Handle might be empty now but it could
        //    be assigned a curve later; use a RelinkableHandle here
        auto tmp = MakeOIS(Period(), clonedOvernightIndex, 0.0, forwardStart)
            .withDiscountingTermStructure(discountRelinkableHandle_)
            .withEffectiveDate(startDate)
            .withTerminationDate(endDate)
            .withTelescopicValueDates(telescopicValueDates_)
            .withPaymentLag(paymentLag)
            .withPaymentAdjustment(paymentConvention)
            .withPaymentFrequency(paymentFrequency)
            .withPaymentCalendar(paymentCalendar)
            .withOvernightLegSpread(overnightSpread)
            .withAveragingMethod(averagingMethod_);
        if (endOfMonth) {
            swap_ = tmp.withEndOfMonth(*endOfMonth);
        } else {
            swap_ = tmp;
        }

        earliestDate_ = swap_->startDate();
        Date lastPaymentDate = std::max(swap_->overnightLeg().back()->date(),
                                        swap_->fixedLeg().back()->date());
        latestDate_ = std::max(swap_->maturityDate(), lastPaymentDate);
    }

    void DatedOISRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        if (discountHandle_.empty())
            discountRelinkableHandle_.linkTo(temp, observer);
        else
            discountRelinkableHandle_.linkTo(*discountHandle_, observer);

        RateHelper::setTermStructure(t);
    }

    Real DatedOISRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->deepUpdate();
        return swap_->fairRate();
    }

    void DatedOISRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<DatedOISRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
