/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2014, 2015 Ferdinando Ametrano
 Copyright (C) 2015 Paolo Mazzocchi

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

using boost::shared_ptr;

namespace QuantLib {

    MakeOIS::MakeOIS(const Period& swapTenor,
                     const shared_ptr<OvernightIndex>& overnightIndex,
                     Rate fixedRate,
                     const Period& forwardStart)
    : swapTenor_(swapTenor), overnightIndex_(overnightIndex),
      fixedRate_(fixedRate), forwardStart_(forwardStart),
      settlementDays_(2),
      calendar_(overnightIndex->fixingCalendar()),
      paymentFrequency_(Annual),
      rule_(DateGeneration::Backward),
      // any value here for endOfMonth_ would not be actually used
      isDefaultEOM_(true),
      type_(OvernightIndexedSwap::Payer), nominal_(1.0),
      overnightSpread_(0.0),
      fixedDayCount_(overnightIndex->dayCounter()) {}

    MakeOIS::operator OvernightIndexedSwap() const {
        shared_ptr<OvernightIndexedSwap> ois = *this;
        return *ois;
    }

    MakeOIS::operator shared_ptr<OvernightIndexedSwap>() const {

        Date startDate;
        if (effectiveDate_ != Date())
            startDate = effectiveDate_;
        else {
            Date refDate = Settings::instance().evaluationDate();
            // if the evaluation date is not a business day
            // then move to the next business day
            refDate = calendar_.adjust(refDate);
            Date spotDate = calendar_.advance(refDate,
                                              settlementDays_*Days);
            startDate = spotDate+forwardStart_;
            if (forwardStart_.length()<0)
                startDate = calendar_.adjust(startDate, Preceding);
            else
                startDate = calendar_.adjust(startDate, Following);
        }

        // OIS end of month default
        bool usedEndOfMonth = 
            isDefaultEOM_ ? calendar_.isEndOfMonth(startDate) : endOfMonth_;

        Date endDate = terminationDate_;
        if (endDate == Date()) {
            if (usedEndOfMonth)
                endDate = calendar_.advance(startDate,
                                            swapTenor_,
                                            ModifiedFollowing,
                                            usedEndOfMonth);
            else
                endDate = startDate + swapTenor_;
        }

        Schedule schedule(startDate, endDate,
                          Period(paymentFrequency_),
                          calendar_,
                          ModifiedFollowing,
                          ModifiedFollowing,
                          rule_,
                          usedEndOfMonth);

        Rate usedFixedRate = fixedRate_;
        if (fixedRate_ == Null<Rate>()) {
            OvernightIndexedSwap temp(type_, nominal_,
                                      schedule,
                                      0.0, // fixed rate
                                      fixedDayCount_,
                                      overnightIndex_, overnightSpread_);
            if (engine_ == 0) {
                Handle<YieldTermStructure> disc =
                                    overnightIndex_->forwardingTermStructure();
                QL_REQUIRE(!disc.empty(),
                           "null term structure set to this instance of " <<
                           overnightIndex_->name());
                bool includeSettlementDateFlows = false;
                shared_ptr<PricingEngine> engine(new
                    DiscountingSwapEngine(disc, includeSettlementDateFlows));
                temp.setPricingEngine(engine);
            } else
                temp.setPricingEngine(engine_);

            usedFixedRate = temp.fairRate();
        }

        shared_ptr<OvernightIndexedSwap> ois(new
            OvernightIndexedSwap(type_, nominal_,
                                 schedule,
                                 usedFixedRate, fixedDayCount_,
                                 overnightIndex_, overnightSpread_));

        if (engine_ == 0) {
            Handle<YieldTermStructure> disc =
                                overnightIndex_->forwardingTermStructure();
            bool includeSettlementDateFlows = false;
            shared_ptr<PricingEngine> engine(new
                DiscountingSwapEngine(disc, includeSettlementDateFlows));
            ois->setPricingEngine(engine);
        } else
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

    MakeOIS& MakeOIS::withSettlementDays(Natural settlementDays) {
        settlementDays_ = settlementDays;
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
                                        const Handle<YieldTermStructure>& d) {
        bool includeSettlementDateFlows = false;
        engine_ = shared_ptr<PricingEngine>(new
            DiscountingSwapEngine(d, includeSettlementDateFlows));
        return *this;
    }

    MakeOIS& MakeOIS::withPricingEngine(
                             const shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

    MakeOIS& MakeOIS::withFixedLegDayCount(const DayCounter& dc) {
        fixedDayCount_ = dc;
        return *this;
    }

    MakeOIS& MakeOIS::withEndOfMonth(bool flag) {
        endOfMonth_ = flag;
        isDefaultEOM_ = false;
        return *this;
    }

    MakeOIS& MakeOIS::withOvernightLegSpread(Spread sp) {
        overnightSpread_ = sp;
        return *this;
    }

}
