
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

    namespace TermStructures {

        void RateHelper::setTermStructure(const TermStructure* t) {
            QL_REQUIRE(t != 0, "null term structure given");
            termStructure_ = t;
        }


        DepositRateHelper::DepositRateHelper(Rate rate, const Date& settlement,
            int n, TimeUnit units, const Handle<Calendar>& calendar,
            bool isAdjusted, bool isModifiedFollowing,
            const Handle<DayCounter>& dayCounter)
        : rate_(rate), settlement_(settlement), n_(n), units_(units),
          calendar_(calendar), isAdjusted_(isAdjusted),
          isModified_(isModifiedFollowing), dayCounter_(dayCounter) {
            maturity_ = settlement_.plus(n_,units_);
            if (isAdjusted_)
                maturity_ = calendar_->roll(maturity_,isModified_);
            yearFraction_ = dayCounter_->yearFraction(settlement_,maturity_);
        }

        double DepositRateHelper::rateError() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            Rate impliedRate = (termStructure_->discount(settlement_) / 
                                termStructure_->discount(maturity_)-1.0) /
                               yearFraction_;
            return rate_-impliedRate;
        }

        double DepositRateHelper::discountGuess() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return termStructure_->discount(settlement_) / 
                   (1.0+rate_*yearFraction_);
        }

        Date DepositRateHelper::maturity() const {
            QL_REQUIRE(termStructure_ != 0, "term structure not set");
            return maturity_;
        }

    }

}
