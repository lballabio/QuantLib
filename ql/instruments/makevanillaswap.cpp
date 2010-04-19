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

#include <ql/instruments/makevanillaswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    MakeVanillaSwap::MakeVanillaSwap(const Period& swapTenor,
                                     const boost::shared_ptr<IborIndex>& index,
                                     Rate fixedRate,
                                     const Period& forwardStart)
    : swapTenor_(swapTenor), iborIndex_(index),
      fixedRate_(fixedRate), forwardStart_(forwardStart),
      effectiveDate_(Date()),
      fixedCalendar_(index->fixingCalendar()),
      floatCalendar_(index->fixingCalendar()),
      type_(VanillaSwap::Payer), nominal_(1.0),
      fixedTenor_(Period(1, Years)), floatTenor_(index->tenor()),
      fixedConvention_(ModifiedFollowing),
      fixedTerminationDateConvention_(ModifiedFollowing),
      floatConvention_(index->businessDayConvention()),
      floatTerminationDateConvention_(index->businessDayConvention()),
      fixedRule_(DateGeneration::Backward), floatRule_(DateGeneration::Backward),
      fixedEndOfMonth_(false), floatEndOfMonth_(false),
      fixedFirstDate_(Date()), fixedNextToLastDate_(Date()),
      floatFirstDate_(Date()), floatNextToLastDate_(Date()),
      floatSpread_(0.0),
      fixedDayCount_(Thirty360(Thirty360::BondBasis)),
      floatDayCount_(index->dayCounter()),
      engine_(new DiscountingSwapEngine(iborIndex_->forwardingTermStructure())) {}

    MakeVanillaSwap::operator VanillaSwap() const {
        boost::shared_ptr<VanillaSwap> swap = *this;
        return *swap;
    }

    MakeVanillaSwap::operator boost::shared_ptr<VanillaSwap>() const {

        Date startDate;
        if (effectiveDate_ != Date())
            startDate = effectiveDate_;
        else {
            Date referenceDate = Settings::instance().evaluationDate();
            Natural fixingDays = iborIndex_->fixingDays();
            Date spotDate = floatCalendar_.advance(referenceDate,
                                                   fixingDays*Days);
            startDate = spotDate+forwardStart_;
        }

        Date endDate;
        if (terminationDate_ != Date())
            endDate = terminationDate_;
        else
            endDate = startDate+swapTenor_;

        Schedule fixedSchedule(startDate, endDate,
                               fixedTenor_, fixedCalendar_,
                               fixedConvention_,
                               fixedTerminationDateConvention_,
                               fixedRule_, fixedEndOfMonth_,
                               fixedFirstDate_, fixedNextToLastDate_);

        Schedule floatSchedule(startDate, endDate,
                               floatTenor_, floatCalendar_,
                               floatConvention_,
                               floatTerminationDateConvention_,
                               floatRule_ , floatEndOfMonth_,
                               floatFirstDate_, floatNextToLastDate_);

        Rate usedFixedRate = fixedRate_;
        if (fixedRate_ == Null<Rate>()) {
            QL_REQUIRE(!iborIndex_->forwardingTermStructure().empty(),
                       "null term structure set to this instance of " <<
                       iborIndex_->name());
            VanillaSwap temp(type_, nominal_,
                             fixedSchedule,
                             0.0, // fixed rate
                             fixedDayCount_,
                             floatSchedule, iborIndex_,
                             floatSpread_, floatDayCount_);
            // ATM on the forecasting curve
            bool includeSettlementDateFlows = false;
            temp.setPricingEngine(boost::shared_ptr<PricingEngine>(new
                DiscountingSwapEngine(iborIndex_->forwardingTermStructure(),
                                      includeSettlementDateFlows)));
            usedFixedRate = temp.fairRate();
        }

        boost::shared_ptr<VanillaSwap> swap(new
            VanillaSwap(type_, nominal_,
                        fixedSchedule,
                        usedFixedRate, fixedDayCount_,
                        floatSchedule,
                        iborIndex_, floatSpread_, floatDayCount_));
        swap->setPricingEngine(engine_);
        return swap;
    }

    MakeVanillaSwap& MakeVanillaSwap::receiveFixed(bool flag) {
        type_ = flag ? VanillaSwap::Receiver : VanillaSwap::Payer ;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withType(VanillaSwap::Type type) {
        type_ = type;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withTerminationDate(const Date& terminationDate) {
        terminationDate_ = terminationDate;
        swapTenor_ = Period();
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withRule(DateGeneration::Rule r) {
        fixedRule_ = r;
        // fixedTenor_ = ??
        floatRule_ = r;
        // floatTenor_ = ??
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withDiscountingTermStructure(
                const Handle<YieldTermStructure>& discountingTermStructure) {
        engine_ = boost::shared_ptr<PricingEngine>(new
                            DiscountingSwapEngine(discountingTermStructure));
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegTenor(const Period& t) {
        fixedTenor_ = t;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegCalendar(const Calendar& cal) {
        fixedCalendar_ = cal;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegConvention(BusinessDayConvention bdc) {
        fixedConvention_ = bdc;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegTerminationDateConvention(BusinessDayConvention bdc) {
        fixedTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegRule(DateGeneration::Rule r) {
        fixedRule_ = r;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegEndOfMonth(bool flag) {
        fixedEndOfMonth_ = flag;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegFirstDate(const Date& d) {
        fixedFirstDate_ = d;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegNextToLastDate(const Date& d) {
        fixedNextToLastDate_ = d;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegDayCount(const DayCounter& dc) {
        fixedDayCount_ = dc;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFloatingLegTenor(const Period& t) {
        floatTenor_ = t;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegCalendar(const Calendar& cal) {
        floatCalendar_ = cal;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegConvention(BusinessDayConvention bdc) {
        floatConvention_ = bdc;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegTerminationDateConvention(BusinessDayConvention bdc) {
        floatTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFloatingLegRule(DateGeneration::Rule r) {
        floatRule_ = r;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFloatingLegEndOfMonth(bool flag) {
        floatEndOfMonth_ = flag;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegFirstDate(const Date& d) {
        floatFirstDate_ = d;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegNextToLastDate(const Date& d) {
        floatNextToLastDate_ = d;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegDayCount(const DayCounter& dc) {
        floatDayCount_ = dc;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFloatingLegSpread(Spread sp) {
        floatSpread_ = sp;
        return *this;
    }

}
