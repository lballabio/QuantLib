
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

/*! \file xibor.cpp
    \brief purely virtual base class for libor indexes

    $Id$
    $Source$
    $Log$
    Revision 1.2  2001/05/24 11:15:57  lballabio
    Stripped conventions from Currencies

    Revision 1.1  2001/05/16 09:57:27  lballabio
    Added indexes and piecewise flat forward curve

*/

#include "ql/Indexes/xibor.hpp"
#include "ql/Indexes/libormanager.hpp"

namespace QuantLib {

    namespace Indexes {

        Rate Xibor::fixing(const Date& fixingDate, 
          int n, TimeUnit unit) const {
            Date settlementDate = termStructure()->settlementDate();
            if (fixingDate < settlementDate) {
                // must have been fixed
                Rate pastFixing = 
                    LiborManager::getHistory(currency(),n,unit)[fixingDate];
                QL_REQUIRE(pastFixing != Null<double>(),
                    "Missing " + CurrencyFormatter::toString(currency()) +
                        " Libor fixing for " + 
                        DateFormatter::toString(fixingDate));
                return pastFixing;
            }
            if (fixingDate == settlementDate) {
                // might have been fixed
                try {
                    Rate pastFixing = 
                        LiborManager::getHistory(currency(),n,unit)[fixingDate];
                    if (pastFixing != Null<double>())
                        return pastFixing;
                    else
                        ;   // fall through and forecast
                } catch (Error&) {
                    ;       // fall through and forecast
                }
            }
            Date endDate = fixingDate.plus(n,unit);
            if (isAdjusted())
                endDate = calendar()->roll(endDate,isModifiedFollowing());
            DiscountFactor fixingDiscount = 
                termStructure()->discount(fixingDate);
            DiscountFactor endDiscount = 
                termStructure()->discount(endDate);
            double fixingPeriod = 
                dayCounter()->yearFraction(fixingDate, endDate);
            return (fixingDiscount/endDiscount-1.0) / fixingPeriod;
        }

        Handle<TermStructure> Xibor::termStructure() const {
            return LiborManager::getTermStructure(currency());
        }

    }

}

