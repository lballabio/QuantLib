
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file ratehelpers.cpp
    \brief rate helpers base class

    \fullpath
    ql/TermStructures/%ratehelpers.cpp
*/

// $Id$

#include <ql/TermStructures/ratehelpers.hpp>

namespace QuantLib {

    using Instruments::SimpleSwap;
    using Indexes::Xibor;

    namespace TermStructures {

        RateHelper::RateHelper(const RelinkableHandle<MarketElement>& quote)
        : quote_(quote), termStructure_(0) {
            registerWith(quote_);
        }

        RateHelper::RateHelper(double quote)
        : quote_(RelinkableHandle<MarketElement>(
            Handle<MarketElement>(new SimpleMarketElement(quote)))),
        termStructure_(0) {
            registerWith(quote_);
        }

        void RateHelper::setTermStructure(TermStructure* t) {
            QL_REQUIRE(t != 0, "RateHelper: null term structure given");
            termStructure_ = t;
        }

        double RateHelper::quoteError() const {
            return quote_->value()-impliedQuote();
        }



        DepositRateHelper::DepositRateHelper(
            const RelinkableHandle<MarketElement>& rate,
            int settlementDays, int n, TimeUnit units,
            const Calendar& calendar, RollingConvention convention,
            const DayCounter& dayCounter)
        : RateHelper(rate), settlementDays_(settlementDays),
          n_(n), units_(units), calendar_(calendar), convention_(convention),
          dayCounter_(dayCounter) {}

        DepositRateHelper::DepositRateHelper(double rate,
            int settlementDays, int n, TimeUnit units,
            const Calendar& calendar, RollingConvention convention,
            const DayCounter& dayCounter)
        : RateHelper(rate), settlementDays_(settlementDays),
          n_(n), units_(units), calendar_(calendar), convention_(convention),
          dayCounter_(dayCounter) {}

        double DepositRateHelper::impliedQuote() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return (termStructure_->discount(settlement_) /
                    termStructure_->discount(maturity_)-1.0) /
                    yearFraction_;
        }

        DiscountFactor DepositRateHelper::discountGuess() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            // extrapolation shouldn't be needed if the input makes sense
            // but we'll play it safe
            return termStructure_->discount(settlement_,true) /
                   (1.0+quote_->value()*yearFraction_);
        }

        void DepositRateHelper::setTermStructure(TermStructure* t) {
            RateHelper::setTermStructure(t);
            settlement_ = calendar_.advance(
                termStructure_->todaysDate(),settlementDays_,Days);
            maturity_ = calendar_.advance(
                settlement_,n_,units_,convention_);
            yearFraction_ = dayCounter_.yearFraction(settlement_,maturity_);
        }

        Date DepositRateHelper::maturity() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return maturity_;
        }



        FraRateHelper::FraRateHelper(
            const RelinkableHandle<MarketElement>& rate,
            int settlementDays, int monthsToStart, int monthsToEnd,
            const Calendar& calendar, RollingConvention convention,
            const DayCounter& dayCounter)
        : RateHelper(rate), settlementDays_(settlementDays),
          monthsToStart_(monthsToStart), monthsToEnd_(monthsToEnd),
          calendar_(calendar), convention_(convention),
          dayCounter_(dayCounter) {}

        FraRateHelper::FraRateHelper(double rate,
            int settlementDays, int monthsToStart, int monthsToEnd,
            const Calendar& calendar, RollingConvention convention,
            const DayCounter& dayCounter)
        : RateHelper(rate), settlementDays_(settlementDays),
          monthsToStart_(monthsToStart), monthsToEnd_(monthsToEnd),
          calendar_(calendar), convention_(convention),
          dayCounter_(dayCounter) {}

        double FraRateHelper::impliedQuote() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return (termStructure_->discount(start_) /
                    termStructure_->discount(maturity_)-1.0) /
                    yearFraction_;
        }

        DiscountFactor FraRateHelper::discountGuess() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            // extrapolation shouldn't be needed if the input makes sense
            // but we'll play it safe
            return termStructure_->discount(start_,true) /
                   (1.0+quote_->value()*yearFraction_);
        }

        void FraRateHelper::setTermStructure(TermStructure* t) {
            RateHelper::setTermStructure(t);
            settlement_ = calendar_.advance(
                termStructure_->todaysDate(),settlementDays_,Days);
            start_ = calendar_.advance(
                settlement_,monthsToStart_,Months,convention_);
            maturity_ = calendar_.advance(
                start_,monthsToEnd_-monthsToStart_,Months,convention_);
            yearFraction_ = dayCounter_.yearFraction(start_,maturity_);
        }

        Date FraRateHelper::maturity() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return maturity_;
        }

        FuturesRateHelper::FuturesRateHelper(
            const RelinkableHandle<MarketElement>& price,
            const Date& ImmDate, int settlementDays, int nMonths,
            const Calendar& calendar, RollingConvention convention,
            const DayCounter& dayCounter)
        : RateHelper(price), ImmDate_(ImmDate),
          settlementDays_(settlementDays), nMonths_(nMonths),
          calendar_(calendar), convention_(convention),
          dayCounter_(dayCounter) {
            maturity_ = calendar_.advance(
                ImmDate_, nMonths_, Months, convention_);
            yearFraction_ = dayCounter_.yearFraction(ImmDate_, maturity_);
        }

        FuturesRateHelper::FuturesRateHelper(double price,
            const Date& ImmDate, int settlementDays, int nMonths,
            const Calendar& calendar, RollingConvention convention,
            const DayCounter& dayCounter)
        : RateHelper(price), ImmDate_(ImmDate),
          settlementDays_(settlementDays), nMonths_(nMonths),
          calendar_(calendar), convention_(convention),
          dayCounter_(dayCounter) {
            maturity_ = calendar_.advance(
                ImmDate_, nMonths_, Months, convention_);
            yearFraction_ = dayCounter_.yearFraction(ImmDate_, maturity_);
        }

        double FuturesRateHelper::impliedQuote() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return 100 * (1.0-(termStructure_->discount(ImmDate_) /
                               termStructure_->discount(maturity_)-1.0) /
                               yearFraction_);
        }

        DiscountFactor FuturesRateHelper::discountGuess() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            // extrapolation shouldn't be needed if the input makes sense
            // but we'll play it safe
            return termStructure_->discount(ImmDate_,true) /
                   (1.0+(100.0-quote_->value())/100.0*yearFraction_);
        }

        Date FuturesRateHelper::maturity() const {
            return maturity_;
        }

        SwapRateHelper::SwapRateHelper(
            const RelinkableHandle<MarketElement>& rate,
            int settlementDays, int lengthInYears,
            const Calendar& calendar, RollingConvention convention,
            int fixedFrequency, bool fixedIsAdjusted,
            const DayCounter& fixedDayCount,
            int floatingFrequency)
        : RateHelper(rate), settlementDays_(settlementDays),
          numberOfUnits_(lengthInYears), units_(Years),
          calendar_(calendar),
          convention_(convention),
          fixedFrequency_(fixedFrequency),
          floatingFrequency_(floatingFrequency),
          fixedIsAdjusted_(fixedIsAdjusted),
          fixedDayCount_(fixedDayCount) {}

        SwapRateHelper::SwapRateHelper(
            const RelinkableHandle<MarketElement>& rate,
            int settlementDays, int numberOfUnits, TimeUnit units,
            const Calendar& calendar, RollingConvention convention,
            int fixedFrequency, bool fixedIsAdjusted,
            const DayCounter& fixedDayCount,
            int floatingFrequency)
        : RateHelper(rate), settlementDays_(settlementDays),
          numberOfUnits_(numberOfUnits), units_(units), calendar_(calendar),
          convention_(convention),
          fixedFrequency_(fixedFrequency),
          floatingFrequency_(floatingFrequency),
          fixedIsAdjusted_(fixedIsAdjusted),
          fixedDayCount_(fixedDayCount) {}

        SwapRateHelper::SwapRateHelper(double rate,
            int settlementDays, int lengthInYears,
            const Calendar& calendar, RollingConvention convention,
            int fixedFrequency, bool fixedIsAdjusted,
            const DayCounter& fixedDayCount,
            int floatingFrequency)
        : RateHelper(rate), settlementDays_(settlementDays),
          numberOfUnits_(lengthInYears), units_(Years), calendar_(calendar),
          convention_(convention),
          fixedFrequency_(fixedFrequency),
          floatingFrequency_(floatingFrequency),
          fixedIsAdjusted_(fixedIsAdjusted),
          fixedDayCount_(fixedDayCount) {}

        SwapRateHelper::SwapRateHelper(double rate,
            int settlementDays, int numberOfUnits, TimeUnit units,
            const Calendar& calendar, RollingConvention convention,
            int fixedFrequency, bool fixedIsAdjusted,
            const DayCounter& fixedDayCount,
            int floatingFrequency)
        : RateHelper(rate), settlementDays_(settlementDays),
          numberOfUnits_(numberOfUnits), units_(units), calendar_(calendar),
          convention_(convention),
          fixedFrequency_(fixedFrequency),
          floatingFrequency_(floatingFrequency),
          fixedIsAdjusted_(fixedIsAdjusted),
          fixedDayCount_(fixedDayCount) {}

        void SwapRateHelper::setTermStructure(TermStructure* t) {
            // do not set the relinkable handle as an observer -
            // force recalculation when needed
            termStructureHandle_.linkTo(
                Handle<TermStructure>(t,false),false);
            RateHelper::setTermStructure(t);
            settlement_ = calendar_.advance(
                termStructure_->todaysDate(),settlementDays_,Days);
            // dummy Libor index with curve/swap parameters
            Handle<Xibor> dummyIndex(new Xibor("dummy",
                12/floatingFrequency_,Months,settlementDays_,
                EUR, // any would do
                calendar_,true,convention_,
                t->dayCounter(),
                termStructureHandle_));
            swap_ = Handle<SimpleSwap>(
                new SimpleSwap(true,                // pay fixed rate
                    settlement_, numberOfUnits_, units_, calendar_,
                    convention_,
                    100.0,
                    fixedFrequency_,
                    0.0,
                    fixedIsAdjusted_, fixedDayCount_,
                    floatingFrequency_, dummyIndex, settlementDays_,
                    0.0,
                    termStructureHandle_));
        }

        Date SwapRateHelper::maturity() const {
            QL_REQUIRE(termStructure_ != 0, "null term structure set");
            return swap_->maturity();
        }

        double SwapRateHelper::impliedQuote() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            // we didn't register as observers - force calculation
            swap_->recalculate();
            return swap_->fairRate();
        }

    }

}
