
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

/*! \file xibor.cpp
    \brief purely virtual base class for libor indexes

    \fullpath
    ql/Indexes/%xibor.cpp
*/

// $Id$

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

