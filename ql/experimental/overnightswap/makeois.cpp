/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2006 StatPro Italia srl

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

#include <ql/experimental/overnightswap/makeois.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    MakeOIS::MakeOIS(const Period& swapTenor,
                     const boost::shared_ptr<OvernightIndex>& overnightIndex,
                     Rate fixedRate,
                     const Date& effectiveDate)
    : swapTenor_(swapTenor), overnightIndex_(overnightIndex),
      fixedRate_(fixedRate), effectiveDate_(effectiveDate),
      type_(OvernightIndexedSwap::Payer), nominal_(1.0),
      fixingDays_(2), paymentFrequency_(Annual),
      rule_(DateGeneration::Backward), endOfMonth_(true),
      overnightSpread_(0.0),
      fixedDayCount_(overnightIndex->dayCounter()),
      engine_(new DiscountingSwapEngine(overnightIndex_->termStructure())) {}

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
            startDate = calendar.advance(referenceDate, fixingDays_*Days);
        }

        Date endDate;
        if (terminationDate_ != Date())
            endDate = terminationDate_;
        else
            endDate = startDate+swapTenor_;

        Schedule schedule(startDate, endDate,
                          Period(paymentFrequency_),
                          calendar,
                          Following,
                          Following,
                          rule_,
                          endOfMonth_);

        Rate usedFixedRate = fixedRate_;
        if (fixedRate_ == Null<Rate>()) {
            QL_REQUIRE(!overnightIndex_->termStructure().empty(),
                       "no forecasting term structure set to " <<
                       overnightIndex_->name());
            OvernightIndexedSwap temp(type_, nominal_,
                                      schedule, 0.0, fixedDayCount_,
                                      overnightIndex_, overnightSpread_);
            // ATM on the forecasting curve
            bool includeSettlementDateFlows = false;
            temp.setPricingEngine(boost::shared_ptr<PricingEngine>(new
                DiscountingSwapEngine(overnightIndex_->termStructure(),
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

    MakeOIS& MakeOIS::withTerminationDate(const Date& terminationDate) {
        terminationDate_ = terminationDate;
        swapTenor_ = Period();
        return *this;
    }

    MakeOIS& MakeOIS::withRule(DateGeneration::Rule r) {
        rule_ = r;
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
