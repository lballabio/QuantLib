
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file piecewiseconstantforwards.cpp
    \brief piecewise constant forward rate term structure

    $Source$
    $Name$
    $Log$
    Revision 1.14  2001/01/18 13:18:27  nando
    now term structure allows extrapolation

    Revision 1.13  2001/01/17 14:37:57  nando
    tabs removed

    Revision 1.12  2000/12/27 14:05:58  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.11  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#include "piecewiseconstantforwards.h"

namespace QuantLib {

    namespace TermStructures {

        PiecewiseConstantForwards::PiecewiseConstantForwards(Handle<Currency> currency, Handle<DayCounter> dayCounter, const Date& today,
          const std::vector<Deposit>& deposits)
        : theCurrency(currency), theDayCounter(dayCounter), today(today), theDeposits(deposits) {
            QL_REQUIRE(theDeposits.size()>0,"No deposits given");
            nodesNumber_ = theDeposits.size()+1;
            theNodes.resize(nodesNumber_);
            theTimes.resize(nodesNumber_);
            theDiscounts.resize(nodesNumber_);
            theForwards.resize(nodesNumber_);
            theZeroYields.resize(nodesNumber_);
            // values at settlement date
            Date settlement = settlementDate();
            theDiscounts[0] = DiscountFactor(1.0);
            theNodes[0] = settlement;
            theTimes[0] = Time(0.0);
            // loop on deposits
            for (int i=1; i<nodesNumber_; i++) {
                Deposit& deposit = theDeposits[i-1];
                theNodes[i] = deposit.maturity();
                theTimes[i] = theDayCounter->yearFraction(settlementDate(),theNodes[i]);
                theDiscounts[i] = 1.0/(1.0+deposit.rate()*deposit.dayCounter()->yearFraction(settlement,theNodes[i]));
                theForwards[i] = QL_LOG(theDiscounts[i-1]/theDiscounts[i])/(theTimes[i]-theTimes[i-1]);
                theZeroYields[i] = -QL_LOG(theDiscounts[i])/theTimes[i];
            }
            // missing values at settlement
            theForwards[0] = theZeroYields[0] = theForwards[1];
        }

        Rate PiecewiseConstantForwards::zeroYield(const Date& d, bool extrapolate) const {
            if (d == settlementDate()) {
                return theZeroYields[0];
            } else {
                int n = nextNode(d, extrapolate);
                if (d == theNodes[n]) {
                    return theZeroYields[n];
                } else {
                    Time t = theDayCounter->yearFraction(settlementDate(),d), tn = theTimes[n-1];
                    return (theZeroYields[n-1]*tn+theForwards[n]*(t-tn))/t;
                }
            }
            QL_DUMMY_RETURN(Rate());
        }

        DiscountFactor PiecewiseConstantForwards::discount(const Date& d, bool extrapolate) const {
            if (d == settlementDate()) {
                return theDiscounts[0];
            } else {
                int n = nextNode(d, extrapolate);
                if (d == theNodes[n]) {
                    return theDiscounts[n];
                } else {
                    Time t = theDayCounter->yearFraction(settlementDate(),d);
                    return theDiscounts[n-1]*QL_EXP(-theForwards[n]*(t-theTimes[n-1]));
                }
            }
            QL_DUMMY_RETURN(DiscountFactor());
        }

        Rate PiecewiseConstantForwards::forward(const Date& d, bool extrapolate) const {
            if (d == settlementDate()) {
                return theForwards[0];
            } else {
                return theForwards[nextNode(d, extrapolate)];
            }
            QL_DUMMY_RETURN(Rate());
        }

        int PiecewiseConstantForwards::nextNode(const Date& d, bool extrapolate) const {

            if (extrapolate && d>maxDate())
                return nodesNumber_;

            QL_REQUIRE(d>=minDate() && d<=maxDate(), "date outside curve definition");

            std::vector<Date>::const_iterator i=theNodes.begin(), j=theNodes.end(), k;
            while (j-i > 1) {
                k = i+(j-i)/2;
                if (d <= *k)
                    j = k;
                else
                    i = k;
            }
            return (j-theNodes.begin());
        }

    }

}
