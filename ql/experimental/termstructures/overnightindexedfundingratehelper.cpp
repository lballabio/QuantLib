/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Kyrylo Protsenko

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/experimental/termstructures/overnightindexedfundingratehelper.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {

    OvernightIndexedFundingRateHelper::OvernightIndexedFundingRateHelper(
        const std::variant<Spread, Handle<Quote>>& margin,
        const Period& tenor,
        Natural settlementDays,
        Calendar calendar,
        BusinessDayConvention convention,
        bool endOfMonth,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
        const Period& paymentTenor,
        DayCounter paymentDayCounter,
        Integer paymentLag,
        bool telescopicValueDates,
        DateGeneration::Rule rule,
        Pillar::Choice pillar,
        Date customPillarDate)
    : RelativeDateRateHelper(margin), tenor_(tenor), settlementDays_(settlementDays),
      calendar_(std::move(calendar)), convention_(convention), endOfMonth_(endOfMonth),
      paymentTenor_(paymentTenor), paymentDayCounter_(std::move(paymentDayCounter)),
      paymentLag_(paymentLag), telescopicValueDates_(telescopicValueDates), rule_(rule),
      pillarChoice_(pillar) {
        initialize(overnightIndex, customPillarDate);
    }

    OvernightIndexedFundingRateHelper::OvernightIndexedFundingRateHelper(
        const std::variant<Spread, Handle<Quote>>& margin,
        const Date& startDate,
        const Date& endDate,
        Calendar calendar,
        BusinessDayConvention convention,
        bool endOfMonth,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
        const Period& paymentTenor,
        DayCounter paymentDayCounter,
        Integer paymentLag,
        bool telescopicValueDates,
        DateGeneration::Rule rule,
        Pillar::Choice pillar,
        Date customPillarDate)
    : RelativeDateRateHelper(margin, false), settlementDays_(Null<Natural>()),
      startDate_(startDate), endDate_(endDate), calendar_(std::move(calendar)),
      convention_(convention), endOfMonth_(endOfMonth), paymentTenor_(paymentTenor),
      paymentDayCounter_(std::move(paymentDayCounter)), paymentLag_(paymentLag),
      telescopicValueDates_(telescopicValueDates), rule_(rule), pillarChoice_(pillar) {
        initialize(overnightIndex, customPillarDate);
    }

    void OvernightIndexedFundingRateHelper::initialize(
        const ext::shared_ptr<OvernightIndex>& overnightIndex, Date customPillarDate) {
        QL_REQUIRE(overnightIndex, "null overnight index");
        QL_REQUIRE(!overnightIndex->forwardingTermStructure().empty(),
                   "the overnight index must have a forwarding curve");
        QL_REQUIRE(paymentTenor_.length() > 0, "non-positive payment tenor");
        QL_REQUIRE(paymentLag_ >= 0, "negative payment lag");

        overnightIndex_ = overnightIndex;
        registerWith(overnightIndex_);

        pillarDate_ = customPillarDate;
        OvernightIndexedFundingRateHelper::initializeDates();
    }

    void OvernightIndexedFundingRateHelper::initializeDates() {
        Date effectiveDate = startDate_;
        Date terminationDate = endDate_;
        if (effectiveDate == Date()) {
            Date today = Settings::instance().evaluationDate();
            effectiveDate = calendar_.advance(today, settlementDays_, Days, Following);
            terminationDate = effectiveDate + tenor_;
        }

        QL_REQUIRE(effectiveDate < terminationDate, "start date must be before end date");

        Schedule schedule = MakeSchedule()
                                .from(effectiveDate)
                                .to(terminationDate)
                                .withTenor(paymentTenor_)
                                .withCalendar(calendar_)
                                .withConvention(convention_)
                                .withTerminationDateConvention(convention_)
                                .endOfMonth(endOfMonth_)
                                .withRule(rule_);

        Leg overnightLeg = OvernightLeg(schedule, overnightIndex_)
                               .withNotionals(100.0)
                               .withPaymentDayCounter(paymentDayCounter_)
                               .withPaymentCalendar(calendar_)
                               .withPaymentAdjustment(Following)
                               .withPaymentLag(paymentLag_)
                               .withTelescopicValueDates(telescopicValueDates_)
                               .withAveragingMethod(RateAveraging::Compound);

        Date initialPaymentDate =
            calendar_.advance(schedule.startDate(), paymentLag_, Days, Following);
        Date finalPaymentDate = overnightLeg.back()->date();
        Leg exchangeLeg = {ext::make_shared<SimpleCashFlow>(100.0, initialPaymentDate),
                           ext::make_shared<SimpleCashFlow>(-100.0, finalPaymentDate)};

        earliestDate_ = schedule.startDate();
        maturityDate_ = schedule.endDate();

        auto lastCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(overnightLeg.back());
        QL_REQUIRE(lastCoupon, "expected an overnight indexed coupon");
        Date lastFixingEndDate =
            overnightIndex_->maturityDate(overnightIndex_->valueDate(lastCoupon->fixingDate()));
        latestRelevantDate_ = std::max(finalPaymentDate, lastFixingEndDate);

        switch (pillarChoice_) {
            case Pillar::MaturityDate:
                pillarDate_ = maturityDate_;
                break;
            case Pillar::LastRelevantDate:
                pillarDate_ = latestRelevantDate_;
                break;
            case Pillar::CustomDate:
                QL_REQUIRE(pillarDate_ >= earliestDate_,
                           "pillar date must not be before the start date");
                QL_REQUIRE(pillarDate_ <= latestRelevantDate_,
                           "pillar date must not be after the latest relevant date");
                break;
            default:
                QL_FAIL("unknown Pillar::Choice(" << Integer(pillarChoice_) << ")");
        }
        latestDate_ = pillarDate_;

        swap_ = ext::make_shared<Swap>(overnightLeg, exchangeLeg);
        swap_->setPricingEngine(
            ext::make_shared<DiscountingSwapEngine>(discountRelinkableHandle_, true));
    }

    void OvernightIndexedFundingRateHelper::setTermStructure(YieldTermStructure* t) {
        bool observer = false;
        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        discountRelinkableHandle_.linkTo(temp, observer);
        RelativeDateRateHelper::setTermStructure(t);
    }

    Real OvernightIndexedFundingRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        swap_->deepUpdate();
        return -(swap_->NPV() / swap_->legBPS(0)) * 1.0e-4;
    }

    void OvernightIndexedFundingRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<OvernightIndexedFundingRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
