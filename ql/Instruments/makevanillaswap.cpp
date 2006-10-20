/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Instruments/makevanillaswap.hpp>
#include <ql/DayCounters/thirty360.hpp>

namespace QuantLib {

    MakeVanillaSwap::MakeVanillaSwap(const Period& swapTenor, 
                                     const boost::shared_ptr<Xibor>& index,
                                     Rate fixedRate)
    : swapTenor_(swapTenor), index_(index), fixedRate_(fixedRate), 
      fixedCalendar_(index->calendar()), floatCalendar_(index->calendar()),
      discountingTermStructure_(index->termStructure()), 
      type_(VanillaSwap::Payer), nominal_(1.0), 
      fixedTenor_(Period(1, Years)), floatTenor_(index->tenor()), 
      fixedConvention_(ModifiedFollowing),
      fixedTerminationDateConvention_(ModifiedFollowing),
      floatConvention_(ModifiedFollowing),
      floatTerminationDateConvention_(ModifiedFollowing),
      fixedBackward_(true), floatBackward_(true), 
      fixedEndOfMonth_(false), floatEndOfMonth_(false),
      fixedFirstDate_(Date()), fixedNextToLastDate_(Date()),
      floatFirstDate_(Date()), floatNextToLastDate_(Date()),
      floatSpread_(0.0), 
      fixedDayCount_(Thirty360()), floatDayCount_(index->dayCounter())
    {
      Integer fixingDays = index_->settlementDays();
      Date referenceDate = Settings::instance().evaluationDate();
      effectiveDate_ = floatCalendar_.advance(referenceDate, 2*Days);
    }

    MakeVanillaSwap::MakeVanillaSwap(const Date& effectiveDate,
                                     const Period& swapTenor, 
                                     const Calendar& cal,
                                     Rate fixedRate,
                                     const boost::shared_ptr<Xibor>& index,
                                     const Handle<YieldTermStructure>& termStructure)
    : swapTenor_(swapTenor), index_(index), fixedRate_(fixedRate),
      effectiveDate_(effectiveDate),
      fixedCalendar_(cal), floatCalendar_(cal), 
      discountingTermStructure_(termStructure), 
      type_(VanillaSwap::Payer), nominal_(1.0), 
      fixedTenor_(Period(1, Years)), floatTenor_(index->tenor()), 
      fixedConvention_(ModifiedFollowing),
      fixedTerminationDateConvention_(ModifiedFollowing),
      floatConvention_(ModifiedFollowing),
      floatTerminationDateConvention_(ModifiedFollowing),
      fixedBackward_(true), floatBackward_(true), 
      fixedEndOfMonth_(false), floatEndOfMonth_(false),
      fixedFirstDate_(Date()), fixedNextToLastDate_(Date()),
      floatFirstDate_(Date()), floatNextToLastDate_(Date()),
      floatSpread_(0.0), 
      fixedDayCount_(Thirty360()), floatDayCount_(index->dayCounter()) {}

    MakeVanillaSwap::operator VanillaSwap() const {

        Date terminationDate = NullCalendar().advance(effectiveDate_, swapTenor_);

        Schedule fixedSchedule(effectiveDate_, terminationDate,
                               fixedTenor_, fixedCalendar_,
                               fixedConvention_,
                               fixedTerminationDateConvention_,
                               fixedBackward_, fixedEndOfMonth_,
                               fixedFirstDate_, fixedNextToLastDate_);

        Schedule floatSchedule(effectiveDate_, terminationDate,
                               floatTenor_, floatCalendar_,
                               floatConvention_,
                               floatTerminationDateConvention_,
                               floatBackward_, floatEndOfMonth_,
                               floatFirstDate_, floatNextToLastDate_);

        return VanillaSwap(type_, nominal_,
                       fixedSchedule, fixedRate_, fixedDayCount_,
                       floatSchedule, index_, floatSpread_, floatDayCount_,
                       discountingTermStructure_);
    }

    MakeVanillaSwap::operator boost::shared_ptr<VanillaSwap>() const {

        Date terminationDate = NullCalendar().advance(effectiveDate_, swapTenor_);

        Schedule fixedSchedule(effectiveDate_, terminationDate,
                               fixedTenor_, fixedCalendar_,
                               fixedConvention_,
                               fixedTerminationDateConvention_,
                               fixedBackward_, fixedEndOfMonth_,
                               fixedFirstDate_, fixedNextToLastDate_);

        Schedule floatSchedule(effectiveDate_, terminationDate,
                               floatTenor_, floatCalendar_,
                               floatConvention_,
                               floatTerminationDateConvention_,
                               floatBackward_, floatEndOfMonth_,
                               floatFirstDate_, floatNextToLastDate_);

        return boost::shared_ptr<VanillaSwap>(new
            VanillaSwap(type_, nominal_,
                   fixedSchedule, fixedRate_, fixedDayCount_,
                   floatSchedule, index_, floatSpread_, floatDayCount_,
                   discountingTermStructure_));
    }

    MakeVanillaSwap& MakeVanillaSwap::receiveFixed(bool flag) {
        type_ = flag ? VanillaSwap::Receiver : VanillaSwap::Payer ;
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

    MakeVanillaSwap& MakeVanillaSwap::withDiscountingTermStructure(
                const Handle<YieldTermStructure>& discountingTermStructure) {
        discountingTermStructure_=discountingTermStructure;
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

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegForward(bool flag) {
        fixedBackward_ = !flag;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegNotEndOfMonth(bool flag) {
        fixedEndOfMonth_ = !flag;
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

    MakeVanillaSwap& MakeVanillaSwap::withFloatingLegForward(bool flag) {
        floatBackward_ = !flag;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegNotEndOfMonth(bool flag) {
        floatEndOfMonth_ = !flag;
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
