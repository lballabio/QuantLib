
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
    
    $Source$
    $Log$
    Revision 1.3  2001/05/17 15:33:30  lballabio
    Deposit rate helpers now use conventions in Currency

    Revision 1.2  2001/05/16 15:45:56  lballabio
    Fixed typo in docs

    Revision 1.1  2001/05/16 09:57:27  lballabio
    Added indexes and piecewise flat forward curve

*/

#include "ql/TermStructures/ratehelpers.hpp"

namespace QuantLib {

    namespace TermStructures {
        
        void DepositRateHelper::setTermStructure(const TermStructure* t) {
            QL_REQUIRE(t != 0, "null term structure given");
            QL_REQUIRE(t->currency() == currency_,
                "Mismatch between deposit currency (" +
                currency_->name() +
                ") and term structure currency (" +
                t->currency()->name() + ")");
            RateHelper::setTermStructure(t);
            maturity_ = termStructure_->settlementDate().plus(n_,units_);
            if (currency_->depositIsAdjusted())
                maturity_ = currency_->settlementCalendar()->roll(
                    maturity_,currency_->depositIsModified());
            yearFraction_ = currency_->depositDayCounter()->yearFraction(
                termStructure_->settlementDate(),maturity_);
        }
        
        double DepositRateHelper::rateError() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            Rate impliedRate = (1.0/termStructure_->discount(maturity_)-1.0) / 
                yearFraction_;
            return rate_-impliedRate;
        }
    
        double DepositRateHelper::discountGuess() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return 1.0/(1.0+rate_*yearFraction_);
        }
    
        Date DepositRateHelper::maturity() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return maturity_;
        }

    }

}
