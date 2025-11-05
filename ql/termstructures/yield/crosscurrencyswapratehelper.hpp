/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2025  Uzair Beg

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sourceforge.net>.  The license is also available online at
 <http://quantlib.org/license.shtml>.

 This helper bootstraps a curve from a fixed/float cross-currency swap quote.
*/

#ifndef quantlib_cross_currency_swap_rate_helper_hpp
#define quantlib_cross_currency_swap_rate_helper_hpp

#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/handle.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/currency.hpp>

namespace QuantLib {

    class CrossCurrencySwapRateHelper : public RelativeDateRateHelper {
      public:
        CrossCurrencySwapRateHelper(
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
            const Handle<YieldTermStructure>& fixedDiscount = Handle<YieldTermStructure>(),
            const Handle<YieldTermStructure>& floatDiscount = Handle<YieldTermStructure>(),
            Natural settlementDays = 2);

        void setTermStructure(YieldTermStructure*) override;
        Real impliedQuote() const override;

        Period tenor() const { return tenor_; }
        const ext::shared_ptr<IborIndex>& floatIndex() const { return floatIndex_; }

      protected:
        void initializeDates() override;

      private:
        Period tenor_;
        Calendar fixedCalendar_;
        Frequency fixedFrequency_;
        BusinessDayConvention fixedConvention_;
        DayCounter fixedDayCount_;
        Currency fixedCurrency_, floatCurrency_;
        ext::shared_ptr<IborIndex> floatIndex_;
        Handle<Quote> fxSpot_;
        Handle<YieldTermStructure> fixedDiscount_, floatDiscount_;
        Natural settlementDays_;
        Date settlementDate_, maturityDate_;
    };

}

#endif
