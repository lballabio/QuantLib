
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file ratehelpers.cpp
    \brief rate helpers

    $Id$
*/

// $Source$
// $Log$
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
// Revision 1.9  2001/05/29 15:12:48  lballabio
// Reintroduced RollingConventions (and redisabled default extrapolation on PFF curve)
//
// Revision 1.8  2001/05/28 14:54:25  lballabio
// Deposit rates are always adjusted
//
// Revision 1.7  2001/05/28 12:52:58  lballabio
// Simplified Instrument interface
//
// Revision 1.6  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.5  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
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



        FraRateHelper::FraRateHelper(Rate rate, const Date& settlement,
            int n, TimeUnit units, const Handle<Calendar>& calendar,
            RollingConvention convention, const Handle<DayCounter>& dayCounter)
        : rate_(rate), settlement_(settlement), n_(n), units_(units),
          calendar_(calendar), convention_(convention), 
          dayCounter_(dayCounter) {
            maturity_ = calendar_->advance(settlement_,n_,units_,convention_);
            yearFraction_ = dayCounter_->yearFraction(settlement_,maturity_);
        }

        double FraRateHelper::rateError() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            Rate impliedRate = (termStructure_->discount(settlement_) / 
                                termStructure_->discount(maturity_)-1.0) /
                               yearFraction_;
            return rate_-impliedRate;
        }

        double FraRateHelper::discountGuess() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            // extrapolation shouldn't be needed if the input makes sense
            // but we'll play it safe
            return termStructure_->discount(settlement_,true) / 
                   (1.0+rate_*yearFraction_);
        }

        Date FraRateHelper::maturity() const {
            return maturity_;
        }



        SwapRateHelper::SwapRateHelper(Rate rate, 
            const Date& startDate, int n, TimeUnit units,
            const Handle<Calendar>& calendar, 
            RollingConvention rollingConvention, 
            int fixedFrequency, 
            bool fixedIsAdjusted, 
            const Handle<DayCounter>& fixedDayCount, 
            int floatingFrequency, 
            const Indexes::Xibor& index, 
            const Handle<DayCounter>& floatingDayCount)
        : rate_(rate) {
            // we don't need to link the index to our own 
            // relinkable term structure handle since it will be used
            // for historical fixings only/
            swap_ = Handle<SimpleSwap>(
                new SimpleSwap(true,                // pay fixed rate
                    startDate, n, units, calendar, rollingConvention, 
                    std::vector<double>(1,100.0),   // nominal
                    fixedFrequency, 
                    std::vector<Rate>(1,0.0),       // coupon rate
                    fixedIsAdjusted, fixedDayCount, 
                    floatingFrequency, index, 
                    std::vector<Spread>(),       // null spread
                    floatingDayCount, 
                    termStructureHandle_));
        }
        
        void SwapRateHelper::setTermStructure(TermStructure* t) {
            termStructureHandle_.linkTo(Handle<TermStructure>(t,false));
            RateHelper::setTermStructure(t);
        }
        
        Date SwapRateHelper::maturity() const {
            return swap_->maturity();
        }

        double SwapRateHelper::rateError() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            Rate impliedRate = -swap_->NPV()/swap_->BPS();
            return rate_-impliedRate;
        }

    }

}
