
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

    \fullpath
    Sources/TermStructures/%ratehelpers.cpp
    \brief rate helpers

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 14:08:42  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.22  2001/08/31 15:23:48  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.21  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.20  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.19  2001/08/07 11:25:56  sigmud
// copyright header maintenance
//
// Revision 1.18  2001/07/25 15:47:30  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.17  2001/06/21 14:30:43  lballabio
// Observability is back
//
// Revision 1.16  2001/06/20 11:52:30  lballabio
// Some observability is back
//
// Revision 1.15  2001/06/18 08:05:59  lballabio
// Reworked indexes and floating rate coupon
//
// Revision 1.14  2001/06/13 16:18:23  lballabio
// Polished rate helper interfaces
//
// Revision 1.13  2001/06/12 13:43:04  lballabio
// Today's date is back into term structures
// Instruments are now constructed with settlement days instead of settlement date
//
// Revision 1.12  2001/06/05 09:35:14  lballabio
// Updated docs to use Doxygen 1.2.8
//
// Revision 1.11  2001/06/04 09:31:39  lballabio
// Using extrapolation in guesses
//
// Revision 1.10  2001/06/01 16:50:17  lballabio
// Term structure on deposits and swaps
//

#include "ql/TermStructures/ratehelpers.hpp"

namespace QuantLib {

    using Instruments::SimpleSwap;
    
    namespace TermStructures {

        void RateHelper::setTermStructure(TermStructure* t) {
            QL_REQUIRE(t != 0, "null term structure given");
            termStructure_ = t;
        }



        DepositRateHelper::DepositRateHelper(Rate rate, int settlementDays,
            int n, TimeUnit units, const Handle<Calendar>& calendar,
            RollingConvention convention, const Handle<DayCounter>& dayCounter)
        : rate_(rate), settlementDays_(settlementDays), n_(n), units_(units),
          calendar_(calendar), convention_(convention), 
          dayCounter_(dayCounter) {}

        double DepositRateHelper::rateError() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            Rate impliedRate = (termStructure_->discount(settlement_) / 
                                termStructure_->discount(maturity_)-1.0) /
                               yearFraction_;
            return rate_-impliedRate;
        }

        double DepositRateHelper::discountGuess() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            // extrapolation shouldn't be needed if the input makes sense
            // but we'll play it safe
            return termStructure_->discount(settlement_,true) / 
                   (1.0+rate_*yearFraction_);
        }

        void DepositRateHelper::setTermStructure(TermStructure* t) {
            RateHelper::setTermStructure(t);
            QL_REQUIRE(termStructure_ != 0, "null term structure set");
            settlement_ = calendar_->advance(
                termStructure_->todaysDate(),settlementDays_,Days);
            maturity_ = calendar_->advance(settlement_,n_,units_,convention_);
            yearFraction_ = dayCounter_->yearFraction(settlement_,maturity_);
        }

        Date DepositRateHelper::maturity() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return maturity_;
        }



        FraRateHelper::FraRateHelper(Rate rate, int settlementDays, 
            int monthsToStart, int monthsToEnd, 
            const Handle<Calendar>& calendar, RollingConvention convention, 
            const Handle<DayCounter>& dayCounter)
        : rate_(rate), settlementDays_(settlementDays), 
          monthsToStart_(monthsToStart), monthsToEnd_(monthsToEnd),
          calendar_(calendar), convention_(convention), 
          dayCounter_(dayCounter) {}

        double FraRateHelper::rateError() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            Rate impliedRate = (termStructure_->discount(start_) / 
                                termStructure_->discount(maturity_)-1.0) /
                               yearFraction_;
            return rate_-impliedRate;
        }

        double FraRateHelper::discountGuess() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            // extrapolation shouldn't be needed if the input makes sense
            // but we'll play it safe
            return termStructure_->discount(start_,true) / 
                   (1.0+rate_*yearFraction_);
        }

        void FraRateHelper::setTermStructure(TermStructure* t) {
            RateHelper::setTermStructure(t);
            QL_REQUIRE(termStructure_ != 0, "null term structure set");
            settlement_ = calendar_->advance(
                termStructure_->todaysDate(),settlementDays_,Days);
            start_ = calendar_->advance(settlement_, monthsToStart_, Months, 
                convention_);
            maturity_ = calendar_->advance(settlement_, monthsToEnd_, Months, 
                convention_);
            yearFraction_ = dayCounter_->yearFraction(start_,maturity_);
        }

        Date FraRateHelper::maturity() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return maturity_;
        }



        SwapRateHelper::SwapRateHelper(Rate rate, 
            int settlementDays, int lengthInYears, 
            const Handle<Calendar>& calendar, 
            RollingConvention rollingConvention, 
            int fixedFrequency, 
            bool fixedIsAdjusted, 
            const Handle<DayCounter>& fixedDayCount, 
            int floatingFrequency)
        : rate_(rate), settlementDays_(settlementDays), 
          lengthInYears_(lengthInYears), calendar_(calendar), 
          rollingConvention_(rollingConvention), 
          fixedFrequency_(fixedFrequency), 
          floatingFrequency_(floatingFrequency),
          fixedIsAdjusted_(fixedIsAdjusted), 
          fixedDayCount_(fixedDayCount) {}
        
        void SwapRateHelper::setTermStructure(TermStructure* t) {
            // do not set the relinkable handle as an observer - 
            // force recalculation when needed
            termStructureHandle_.linkTo(Handle<TermStructure>(t,false),false);
            RateHelper::setTermStructure(t);
            QL_REQUIRE(termStructure_ != 0, "null term structure set");
            settlement_ = calendar_->advance(
                termStructure_->todaysDate(),settlementDays_,Days);
            // we don't need to link the index to our own 
            // relinkable term structure handle since it will be used
            // for historical fixings only
            swap_ = Handle<SimpleSwap>(
                new SimpleSwap(true,                // pay fixed rate
                    settlement_, lengthInYears_, Years, calendar_, 
                    rollingConvention_, 
                    std::vector<double>(1,100.0),   // nominal
                    fixedFrequency_, 
                    std::vector<Rate>(1,0.0),       // coupon rate
                    fixedIsAdjusted_, fixedDayCount_, 
                    floatingFrequency_, Handle<Index>(), 
                    std::vector<Spread>(),       // null spread
                    termStructureHandle_));
        }
        
        Date SwapRateHelper::maturity() const {
            QL_REQUIRE(termStructure_ != 0, "null term structure set");
            return swap_->maturity();
        }

        double SwapRateHelper::rateError() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            // we didn't register as observers - force calculation
            swap_->recalculate();
            Rate impliedRate = -swap_->NPV()/swap_->BPS();
            return rate_-impliedRate;
        }

    }

}
