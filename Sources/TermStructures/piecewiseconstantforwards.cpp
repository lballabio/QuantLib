
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/

/*! \file piecewiseconstantforwards.cpp
    \brief piecewise constant forward rate term structure

    $Id$
*/

// $Source$
// $Log$
// Revision 1.26  2001/07/25 15:47:30  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.25  2001/06/12 13:43:04  lballabio
// Today's date is back into term structures
// Instruments are now constructed with settlement days instead of settlement date
//
// Revision 1.24  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/TermStructures/piecewiseconstantforwards.hpp"

namespace QuantLib {

    namespace TermStructures {

        PiecewiseConstantForwards::PiecewiseConstantForwards(
            Currency currency, const Handle<DayCounter>& dayCounter,
            const Date& todaysDate, const Handle<Calendar>& calendar, 
            int settlementDays, const std::vector<DepositRate>& deposits)
        : currency_(currency), dayCounter_(dayCounter),
          todaysDate_(todaysDate), calendar_(calendar), 
          settlementDays_(settlementDays), deposits_(deposits) {

            settlementDate_ = calendar_->advance(
                todaysDate_,settlementDays_,Days);
            nodesNumber_ = deposits_.size()+1;
            QL_REQUIRE(deposits_.size()>1, "No deposits given");
            nodes_.resize(nodesNumber_);
            times_.resize(nodesNumber_);
            discounts_.resize(nodesNumber_);
            forwards_.resize(nodesNumber_);
            zeroYields_.resize(nodesNumber_);
            // values at settlement date
            discounts_[0] = DiscountFactor(1.0);
            nodes_[0] = settlementDate_;
            times_[0] = Time(0.0);
            // loop on deposits_
            for (int i=1; i<nodesNumber_; i++) {
                DepositRate& deposit = deposits_[i-1];
                nodes_[i] = deposit.maturity();
                times_[i] = dayCounter_ ->yearFraction(
                    settlementDate_,nodes_[i]);
                discounts_[i] = 1.0/(1.0+deposit.rate() *
                    deposit.dayCounter()->yearFraction(
                        settlementDate_,nodes_[i]));
                forwards_[i] = QL_LOG(discounts_[i-1]/discounts_[i]) /
                                                    (times_[i]-times_[i-1]);
                zeroYields_[i] = -QL_LOG(discounts_[i])/times_[i];
            }
            // missing values at settlement
            forwards_[0] = zeroYields_[0] = forwards_[1];
        }

        Rate PiecewiseConstantForwards::zeroYield(const Date& d,
                                                  bool extrapolate) const {
            if (d == settlementDate_) {
                return zeroYields_[0];
            } else {
                int n = nextNode_(d, extrapolate);
                if (d == nodes_[n]) {
                    return zeroYields_[n];
                } else {
                    Time t = dayCounter_->yearFraction(settlementDate_,d);
                    Time tn = times_[n-1];
                    return (zeroYields_[n-1]*tn+forwards_[n]*(t-tn))/t;
                }
            }
            QL_DUMMY_RETURN(Rate());
        }

        DiscountFactor PiecewiseConstantForwards::discount(
                                                    const Date& d,
                                                    bool extrapolate) const {
            if (d == settlementDate_) {
                return discounts_[0];
            } else {
                int n = nextNode_(d, extrapolate);
                if (d == nodes_[n]) {
                    return discounts_[n];
                } else {
                    Time t = dayCounter_->yearFraction(settlementDate_,d);
                    return discounts_[n-1]*QL_EXP(-forwards_[n] *
                                                        (t-times_[n-1]));
                }
            }
            QL_DUMMY_RETURN(DiscountFactor());
        }

        Rate PiecewiseConstantForwards::forward(const Date& d,
                                                bool extrapolate) const {
            if (d == settlementDate_) {
                return forwards_[0];
            } else {
                return forwards_[nextNode_(d, extrapolate)];
            }
            QL_DUMMY_RETURN(Rate());
        }

        int PiecewiseConstantForwards::nextNode_(const Date& d,
                                                bool extrapolate) const {

            if (extrapolate && d>maxDate())
                return nodesNumber_-1;

            QL_REQUIRE(d>=minDate() && d<=maxDate(),
                    "date outside curve definition");

            std::vector<Date>::const_iterator i=nodes_.begin(),
                                                j=nodes_.end(), k;
            while (j-i > 1) {
                k = i+(j-i)/2;
                if (d <= *k)
                    j = k;
                else
                    i = k;
            }
            return (j-nodes_.begin());
        }

    }

}
