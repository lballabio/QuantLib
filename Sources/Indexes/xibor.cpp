
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
*/

// $Source$
// $Log$
// Revision 1.9  2001/06/18 11:53:42  lballabio
// Fixed name used for history lookup
//
// Revision 1.8  2001/06/15 13:52:07  lballabio
// Reworked indexes
//
// Revision 1.7  2001/06/12 15:05:34  lballabio
// Renamed Libor to GBPLibor and LiborManager to XiborManager
//
// Revision 1.6  2001/06/01 16:50:16  lballabio
// Term structure on deposits and swaps
//
// Revision 1.5  2001/05/29 15:12:48  lballabio
// Reintroduced RollingConventions (and redisabled default extrapolation on PFF curve)
//
// Revision 1.4  2001/05/29 09:24:06  lballabio
// Using relinkable handle to term structure
//
// Revision 1.3  2001/05/24 15:40:09  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Indexes/xibor.hpp"
#include "ql/Indexes/xibormanager.hpp"

namespace QuantLib {

    namespace Indexes {

        std::string Xibor::name() const {
            switch (units_) {
              case Days:
                return familyName_+IntegerFormatter::toString(n_)+"d";
              case Weeks:
                return familyName_+IntegerFormatter::toString(n_)+"w";
              case Months:
                return familyName_+IntegerFormatter::toString(n_)+"m";
              case Years:
                return familyName_+IntegerFormatter::toString(n_)+"y";
              default:
                throw Error("invalid time unit");
            }
            QL_DUMMY_RETURN(std::string());
        }
        
        Rate Xibor::fixing(const Date& fixingDate) const {
            QL_REQUIRE(!termStructure_.isNull(),
                "null term structure set");
            Date settlementDate = termStructure_->settlementDate();
            if (fixingDate < settlementDate) {
                // must have been fixed
                Rate pastFixing =
                    XiborManager::getHistory(name())[fixingDate];
                QL_REQUIRE(pastFixing != Null<double>(),
                    "Missing " + name() + " fixing for " +
                        DateFormatter::toString(fixingDate));
                return pastFixing;
            }
            if (fixingDate == settlementDate) {
                // might have been fixed
                try {
                    Rate pastFixing =
                        XiborManager::getHistory(name())[fixingDate];
                    if (pastFixing != Null<double>())
                        return pastFixing;
                    else
                        ;   // fall through and forecast
                } catch (Error&) {
                    ;       // fall through and forecast
                }
            }
            Date endDate = fixingDate.plus(n_,units_);
            if (isAdjusted_)
                endDate = calendar_->roll(endDate,rollingConvention_);
            DiscountFactor fixingDiscount =
                termStructure_->discount(fixingDate);
            DiscountFactor endDiscount =
                termStructure_->discount(endDate);
            double fixingPeriod =
                dayCounter_->yearFraction(fixingDate, endDate);
            return (fixingDiscount/endDiscount-1.0) / fixingPeriod;
        }

    }

}

