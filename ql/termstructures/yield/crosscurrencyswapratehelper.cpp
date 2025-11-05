/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025  Uzair Beg

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

#include <ql/termstructures/yield/crosscurrencyswapratehelper.hpp>

#include <ql/settings.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/currency.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/quotes/simplequote.hpp>
#include <algorithm>

namespace QuantLib {

    CrossCurrencySwapRateHelper::CrossCurrencySwapRateHelper(
        const Handle<Quote>& fixedRate,
        const Period& tenor,
        const Calendar& fixedCalendar,
        Frequency fixedFrequency,
        BusinessDayConvention fixedConvention,
        const DayCounter& fixedDayCount,
        const Currency& fixedCurrency,
        const ext::shared_ptr<IborIndex>& floatIndex,
        const Currency& floatCurrency,
        const Handle<Quote>& fxSpot,
        const Handle<YieldTermStructure>& fixedDiscount,
        const Handle<YieldTermStructure>& floatDiscount,
        Natural settlementDays)
    : RelativeDateRateHelper(fixedRate),
      tenor_(tenor),
      fixedCalendar_(fixedCalendar),
      fixedFrequency_(fixedFrequency),
      fixedConvention_(fixedConvention),
      fixedDayCount_(fixedDayCount),
      fixedCurrency_(fixedCurrency),
      floatCurrency_(floatCurrency),
      floatIndex_(floatIndex),
      fxSpot_(fxSpot),
      fixedDiscount_(fixedDiscount),
      floatDiscount_(floatDiscount),
      settlementDays_(settlementDays) {

        QL_REQUIRE(floatIndex_, "floating index required");
        QL_REQUIRE(!fxSpot_.empty(), "FX spot quote required");
        QL_REQUIRE(fxSpot_->isValid(), "FX spot quote not valid");

        initializeDates();
    }

    void CrossCurrencySwapRateHelper::initializeDates() {
        const Date refDate = Settings::instance().evaluationDate();
        QL_REQUIRE(refDate != Date(), "evaluation date not set");

        settlementDate_ = fixedCalendar_.advance(refDate, settlementDays_, Days);
        maturityDate_   = fixedCalendar_.advance(settlementDate_, tenor_, fixedConvention_);

        earliestDate_ = settlementDate_;
        latestDate_   = maturityDate_;
    }

    void CrossCurrencySwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // Attach the bootstrap curve 
        RelativeDateRateHelper::setTermStructure(t);
    }

    // Helper: get a discount factor from the *fixed* side curve 
    static inline DiscountFactor dfFromFixedSide(const YieldTermStructure* tsPtr,
                                                 const Handle<YieldTermStructure>& fixedDiscount,
                                                 const Date& d) {
        if (!fixedDiscount.empty())
            return fixedDiscount->discount(d);
        QL_REQUIRE(tsPtr, "internal term structure not set");
        return tsPtr->discount(d);
    }

    // Helper: get a discount factor for the *float* side
    static inline DiscountFactor dfFromFloatSide(const Handle<YieldTermStructure>& floatDiscount,
                                                 const ext::shared_ptr<IborIndex>& idx,
                                                 const Date& d) {
        if (!floatDiscount.empty())
            return floatDiscount->discount(d);
        // fallback: use forwarding curve as discount curve if no explicit float discount provided
        Handle<YieldTermStructure> fwd = idx->forwardingTermStructure();
        QL_REQUIRE(!fwd.empty(), "floating index must have a forwarding curve or provide floatDiscount");
        return fwd->discount(d);
    }

    Real CrossCurrencySwapRateHelper::impliedQuote() const {

        QL_REQUIRE(fxSpot_->isValid(), "invalid FX spot");
        const Real fx = fxSpot_->value();

        const Schedule fixedSched(
            settlementDate_, maturityDate_,
            Period(fixedFrequency_),
            fixedCalendar_,
            fixedConvention_, fixedConvention_,
            DateGeneration::Forward, false);

        const std::vector<Date>& fd = fixedSched.dates();
        QL_REQUIRE(fd.size() >= 2, "fixed schedule has too few dates");

        const Calendar floatCal   = floatIndex_->fixingCalendar();
        const BusinessDayConvention floatBdc = floatIndex_->businessDayConvention();
        const Period floatTenor   = floatIndex_->tenor();

        const Schedule floatSched(
            settlementDate_, maturityDate_,
            floatTenor, floatCal, floatBdc, floatBdc,
            DateGeneration::Forward, false);

        const std::vector<Date>& fl = floatSched.dates();
        QL_REQUIRE(fl.size() >= 2, "floating schedule has too few dates");

        Real pvbp = 0.0;
        for (Size i = 1; i < fd.size(); ++i) {
            const Date accrualStart = fd[i-1];
            const Date accrualEnd   = fd[i];
            const Time accrual = fixedDayCount_.yearFraction(accrualStart, accrualEnd);
            const Date payDate = accrualEnd; // standard conv: pay at period end
            const DiscountFactor df = dfFromFixedSide(termStructure_, fixedDiscount_, payDate);
            pvbp += accrual * df;
        }
        QL_REQUIRE(pvbp > 0.0, "fixed-leg PVBP is non-positive");

        Real pvFloatInFloatCcy = 0.0;
        for (Size j = 1; j < fl.size(); ++j) {
            const Date accrualStart = fl[j-1];
            const Date accrualEnd   = fl[j];
            const Time accrual = floatIndex_->dayCounter().yearFraction(accrualStart, accrualEnd);

            const Date fixingDate = floatIndex_->fixingDate(accrualStart);
            const Rate fwd = floatIndex_->forecastFixing(fixingDate);

            const Date payDate = accrualEnd; 
            const DiscountFactor dfFloat = dfFromFloatSide(floatDiscount_, floatIndex_, payDate);

            pvFloatInFloatCcy += fwd * accrual * dfFloat;
        }

        const Real pvFloatInFixed = pvFloatInFloatCcy * fx;

        const Real fairRate = pvFloatInFixed / pvbp;


        return fairRate;
    }

} 
