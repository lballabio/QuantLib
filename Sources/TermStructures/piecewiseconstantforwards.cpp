
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

/*! \file piecewiseconstantforwards.cpp
    \brief piecewise constant forward rate term structure

    $Source$
    $Name$
    $Log$
    Revision 1.21  2001/04/09 14:13:34  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.20  2001/04/06 18:46:22  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.19  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.18  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.17  2001/01/18 16:22:28  nando
    deposit file and class renamed to DepositRate

    Revision 1.16  2001/01/18 15:51:37  nando
    bug fixed.
    Out of bound array access in case of extrapolation

    Revision 1.15  2001/01/18 14:36:47  nando
    80 columns enforced
    private members with trailing underscore

    Revision 1.14  2001/01/18 13:18:27  nando
    now term structure allows extrapolation

    Revision 1.13  2001/01/17 14:37:57  nando
    tabs removed

    Revision 1.12  2000/12/27 14:05:58  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.11  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#include "ql/TermStructures/piecewiseconstantforwards.hpp"

namespace QuantLib {

    namespace TermStructures {

        PiecewiseConstantForwards::PiecewiseConstantForwards(
                                    Handle<Currency> currency,
                                    Handle<DayCounter> dayCounter,
                                    const Date& today,
                                    const std::vector<DepositRate>& deposits)
        : currency_(currency), dayCounter_(dayCounter), today_(today),
          deposits_(deposits) {

            nodesNumber_ = deposits_.size()+1;
            QL_REQUIRE(deposits_.size()>1, "No deposits given");
            nodes_.resize(nodesNumber_);
            times_.resize(nodesNumber_);
            discounts_.resize(nodesNumber_);
            forwards_.resize(nodesNumber_);
            zeroYields_.resize(nodesNumber_);
            // values at settlement date
            Date settlement = settlementDate();
            discounts_[0] = DiscountFactor(1.0);
            nodes_[0] = settlement;
            times_[0] = Time(0.0);
            // loop on deposits_
            for (int i=1; i<nodesNumber_; i++) {
                DepositRate& deposit = deposits_[i-1];
                nodes_[i] = deposit.maturity();
                times_[i] = dayCounter_ ->yearFraction(settlementDate(),
                                                                  nodes_[i]);
                discounts_[i] = 1.0/(1.0+deposit.rate() *
                    deposit.dayCounter()->yearFraction(settlement,nodes_[i]));
                forwards_[i] = QL_LOG(discounts_[i-1]/discounts_[i]) /
                                                    (times_[i]-times_[i-1]);
                zeroYields_[i] = -QL_LOG(discounts_[i])/times_[i];
            }
            // missing values at settlement
            forwards_[0] = zeroYields_[0] = forwards_[1];
        }

        Rate PiecewiseConstantForwards::zeroYield(const Date& d,
                                                  bool extrapolate) const {
            if (d == settlementDate()) {
                return zeroYields_[0];
            } else {
                int n = nextNode_(d, extrapolate);
                if (d == nodes_[n]) {
                    return zeroYields_[n];
                } else {
                    Time t = dayCounter_->yearFraction(settlementDate(),d);
                    Time tn = times_[n-1];
                    return (zeroYields_[n-1]*tn+forwards_[n]*(t-tn))/t;
                }
            }
            QL_DUMMY_RETURN(Rate());
        }

        DiscountFactor PiecewiseConstantForwards::discount(
                                                    const Date& d,
                                                    bool extrapolate) const {
            if (d == settlementDate()) {
                return discounts_[0];
            } else {
                int n = nextNode_(d, extrapolate);
                if (d == nodes_[n]) {
                    return discounts_[n];
                } else {
                    Time t = dayCounter_->yearFraction(settlementDate(),d);
                    return discounts_[n-1]*QL_EXP(-forwards_[n] *
                                                        (t-times_[n-1]));
                }
            }
            QL_DUMMY_RETURN(DiscountFactor());
        }

        Rate PiecewiseConstantForwards::forward(const Date& d,
                                                bool extrapolate) const {
            if (d == settlementDate()) {
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
