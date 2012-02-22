/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano

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
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    MakeOIS::MakeOIS(const Period& swapTenor,
                     const boost::shared_ptr<OvernightIndex>& overnightIndex,
                     Rate fixedRate,
                     const Period& forwardStart)
    : swapTenor_(swapTenor), overnightIndex_(overnightIndex),
      fixedRate_(fixedRate), forwardStart_(forwardStart),
      fixingDays_(2), paymentFrequency_(Annual),
      rule_(DateGeneration::Forward),
      endOfMonth_(1*Months<=swapTenor && swapTenor<=2*Years ? true : false),
      type_(OvernightIndexedSwap::Payer), nominal_(1.0),
      overnightSpread_(0.0),
      fixedDayCount_(overnightIndex->dayCounter()),
      engine_(new DiscountingSwapEngine(overnightIndex_->forwardingTermStructure())) {}

    MakeOIS::operator OvernightIndexedSwap() const {
        boost::shared_ptr<OvernightIndexedSwap> ois = *this;
        return *ois;
    }

    MakeOIS::operator boost::shared_ptr<OvernightIndexedSwap>() const {

        const Calendar& calendar = overnightIndex_->fixingCalendar();

        Date startDate;
        if (effectiveDate_ != Date())
            startDate = effectiveDate_;
        else {
            Date referenceDate = Settings::instance().evaluationDate();
            Date spotDate = calendar.advance(referenceDate,
                                             fixingDays_*Days);
            startDate = spotDate+forwardStart_;
        }

        Date endDate;
        if (terminationDate_ != Date()) {
            endDate = terminationDate_;
        } else {
            if (endOfMonth_) {
                endDate = calendar.advance(startDate, swapTenor_,
                                           ModifiedFollowing,
                                           endOfMonth_);
            } else {
                endDate = startDate+swapTenor_;
            }
        }

        Schedule schedule(startDate, endDate,
                          Period(paymentFrequency_),
                          calendar,
                          ModifiedFollowing,
                          ModifiedFollowing,
                          rule_,
                          endOfMonth_);

        Rate usedFixedRate = fixedRate_;
        if (fixedRate_ == Null<Rate>()) {
            QL_REQUIRE(!overnightIndex_->forwardingTermStructure().empty(),
                       "null term structure set to this instance of " <<
                       overnightIndex_->name());
            OvernightIndexedSwap temp(type_, nominal_,
                                      schedule,
                                      0.0, // fixed rate
                                      fixedDayCount_,
                                      overnightIndex_, overnightSpread_);
            // ATM on the forecasting curve
            bool includeSettlementDateFlows = false;
            temp.setPricingEngine(boost::shared_ptr<PricingEngine>(
                new DiscountingSwapEngine(
                                   overnightIndex_->forwardingTermStructure(),
                                   includeSettlementDateFlows)));
            usedFixedRate = temp.fairRate();
        }

        boost::shared_ptr<OvernightIndexedSwap> ois(new
            OvernightIndexedSwap(type_, nominal_,
                                 schedule,
                                 usedFixedRate, fixedDayCount_,
                                 overnightIndex_, overnightSpread_));
        ois->setPricingEngine(engine_);
        return ois;
    }

    MakeOIS& MakeOIS::receiveFixed(bool flag) {
        type_ = flag ? OvernightIndexedSwap::Receiver : OvernightIndexedSwap::Payer ;
        return *this;
    }

    MakeOIS& MakeOIS::withType(OvernightIndexedSwap::Type type) {
        type_ = type;
        return *this;
    }

    MakeOIS& MakeOIS::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    MakeOIS& MakeOIS::withSettlementDays(Natural fixingDays) {
        fixingDays_ = fixingDays;
        effectiveDate_ = Date();
        return *this;
    }

    MakeOIS& MakeOIS::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeOIS& MakeOIS::withTerminationDate(const Date& terminationDate) {
        terminationDate_ = terminationDate;
        swapTenor_ = Period();
        return *this;
    }

    MakeOIS& MakeOIS::withPaymentFrequency(Frequency f) {
        paymentFrequency_ = f;
        if (paymentFrequency_==Once)
            rule_ = DateGeneration::Zero;
        return *this;
    }

    MakeOIS& MakeOIS::withRule(DateGeneration::Rule r) {
        rule_ = r;
        if (r==DateGeneration::Zero)
            paymentFrequency_ = Once;
        return *this;
    }

    MakeOIS& MakeOIS::withDiscountingTermStructure(
                const Handle<YieldTermStructure>& discountingTermStructure) {
        engine_ = boost::shared_ptr<PricingEngine>(new
                            DiscountingSwapEngine(discountingTermStructure));
        return *this;
    }

    MakeOIS& MakeOIS::withFixedLegDayCount(const DayCounter& dc) {
        fixedDayCount_ = dc;
        return *this;
    }

    MakeOIS& MakeOIS::withEndOfMonth(bool flag) {
        endOfMonth_ = flag;
        return *this;
    }

    MakeOIS& MakeOIS::withOvernightLegSpread(Spread sp) {
        overnightSpread_ = sp;
        return *this;
    }

}
