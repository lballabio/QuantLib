
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file ratehelpers.cpp
    \brief rate helpers base class

    \fullpath
    ql/TermStructures/%ratehelpers.cpp
*/

// $Id$

#include "ql/TermStructures/ratehelpers.hpp"
#include "ql/Indexes/euribor.hpp"

namespace QuantLib {

    using Instruments::SimpleSwap;
    using Indexes::Xibor;
    using Indexes::Euribor;
    
    namespace TermStructures {

        RateHelper::RateHelper(const RelinkableHandle<MarketElement>& rate) 
        : rate_(rate), termStructure_(0) {
            rate_.registerObserver(this);
        }

        RateHelper::~RateHelper() {
            rate_.unregisterObserver(this);
        }

        void RateHelper::setTermStructure(TermStructure* t) {
            QL_REQUIRE(t != 0, "null term structure given");
            termStructure_ = t;
        }

        Rate RateHelper::rateError() const {
            return rate_->value()-impliedRate();
        }



        DepositRateHelper::DepositRateHelper(
            const RelinkableHandle<MarketElement>& rate, 
            int settlementDays, int n, TimeUnit units, 
            const Handle<Calendar>& calendar, RollingConvention convention, 
            const Handle<DayCounter>& dayCounter)
        : RateHelper(rate), settlementDays_(settlementDays), 
          n_(n), units_(units), calendar_(calendar), convention_(convention), 
          dayCounter_(dayCounter) {}

        Rate DepositRateHelper::impliedRate() const {
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
                   (1.0+rate_->value()*yearFraction_);
        }

        void DepositRateHelper::setTermStructure(TermStructure* t) {
            RateHelper::setTermStructure(t);
            QL_REQUIRE(termStructure_ != 0, "null term structure set");
            settlement_ = calendar_->advance(
                termStructure_->todaysDate(),settlementDays_,Days);
            maturity_ = calendar_->advance(
                settlement_,n_,units_,convention_);
            yearFraction_ = dayCounter_->yearFraction(settlement_,maturity_);
        }

        Date DepositRateHelper::maturity() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return maturity_;
        }



        FraRateHelper::FraRateHelper(
            const RelinkableHandle<MarketElement>& rate, 
            int settlementDays, int monthsToStart, int monthsToEnd, 
            const Handle<Calendar>& calendar, RollingConvention convention, 
            const Handle<DayCounter>& dayCounter)
        : RateHelper(rate), settlementDays_(settlementDays), 
          monthsToStart_(monthsToStart), monthsToEnd_(monthsToEnd),
          calendar_(calendar), convention_(convention), 
          dayCounter_(dayCounter) {}

        Rate FraRateHelper::impliedRate() const {
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
                   (1.0+rate_->value()*yearFraction_);
        }

        void FraRateHelper::setTermStructure(TermStructure* t) {
            RateHelper::setTermStructure(t);
            QL_REQUIRE(termStructure_ != 0, "null term structure set");
            settlement_ = calendar_->advance(
                termStructure_->todaysDate(),settlementDays_,Days);
            start_ = calendar_->advance(
                settlement_,monthsToStart_,Months,convention_);
            maturity_ = calendar_->advance(
                settlement_,monthsToEnd_,Months,convention_);
            yearFraction_ = dayCounter_->yearFraction(start_,maturity_);
        }

        Date FraRateHelper::maturity() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return maturity_;
        }

        FuturesRateHelper::FuturesRateHelper(
            const RelinkableHandle<MarketElement>& price,
            const Date& ImmDate, int settlementDays, int nMonths,
            const Handle<Calendar>& calendar, RollingConvention convention,
            const Handle<DayCounter>& dayCounter)
        : RateHelper(price), ImmDate_(ImmDate),
          settlementDays_(settlementDays), nMonths_(nMonths),
          calendar_(calendar), convention_(convention), 
          dayCounter_(dayCounter) {
            maturity_ = calendar_->advance(
                ImmDate_, nMonths, Months, convention_);
            yearFraction_ = dayCounter_->yearFraction(ImmDate_, maturity_);
        }

        Rate FuturesRateHelper::impliedRate() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return (termStructure_->discount(ImmDate_) / 
                    termStructure_->discount(maturity_)-1.0) /
                    yearFraction_;
        }

        DiscountFactor FuturesRateHelper::discountGuess() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            // extrapolation shouldn't be needed if the input makes sense
            // but we'll play it safe
            return termStructure_->discount(ImmDate_,true) / 
                   (1.0+(100.0-rate_->value())/100.0*yearFraction_);
        }

        void FuturesRateHelper::setTermStructure(TermStructure* t) {
            RateHelper::setTermStructure(t);
            QL_REQUIRE(termStructure_ != 0, "null term structure set");
            settlement_ = calendar_->advance(
                termStructure_->todaysDate(),settlementDays_,Days);
        }

        Date FuturesRateHelper::maturity() const {
            return maturity_;
        }

        SwapRateHelper::SwapRateHelper(
            const RelinkableHandle<MarketElement>& rate, 
            int settlementDays, int lengthInYears, 
            const Handle<Calendar>& calendar, RollingConvention convention, 
            int fixedFrequency, bool fixedIsAdjusted, 
            const Handle<DayCounter>& fixedDayCount, 
            int floatingFrequency)
        : RateHelper(rate), settlementDays_(settlementDays), 
          lengthInYears_(lengthInYears), calendar_(calendar), 
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
            QL_REQUIRE(termStructure_ != 0, "null term structure set");
            settlement_ = calendar_->advance(
                termStructure_->todaysDate(),settlementDays_,Days);
            // any index would do and will give the same results - 
            // it must be a concrete one, though
            Handle<Xibor> dummyIndex(new Euribor(6,Months,
                RelinkableHandle<TermStructure>()));
            swap_ = Handle<SimpleSwap>(
                new SimpleSwap(true,                // pay fixed rate
                    settlement_, lengthInYears_, Years, calendar_, 
                    convention_, 
                    std::vector<double>(1,100.0),   // nominal
                    fixedFrequency_, 
                    std::vector<Rate>(1,0.0),       // coupon rate
                    fixedIsAdjusted_, fixedDayCount_, 
                    floatingFrequency_, dummyIndex, 
                    std::vector<Spread>(),       // null spread
                    termStructureHandle_));
        }
        
        Date SwapRateHelper::maturity() const {
            QL_REQUIRE(termStructure_ != 0, "null term structure set");
            return swap_->maturity();
        }

        Rate SwapRateHelper::impliedRate() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            // we didn't register as observers - force calculation
            swap_->recalculate();
            return -swap_->NPV()/swap_->fixedLegBPS();
        }

    }

}
