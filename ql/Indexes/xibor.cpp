

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file xibor.cpp
    \brief purely virtual base class for libor indexes

    \fullpath
    ql/Indexes/%xibor.cpp
*/

// $Id$

#include <ql/Indexes/xibor.hpp>
#include <ql/Indexes/xibormanager.hpp>

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
                endDate = calendar_.roll(endDate,rollingConvention_);
            DiscountFactor fixingDiscount =
                termStructure_->discount(fixingDate);
            DiscountFactor endDiscount =
                termStructure_->discount(endDate);
            double fixingPeriod =
                dayCounter().yearFraction(fixingDate, endDate);
            return (fixingDiscount/endDiscount-1.0) / fixingPeriod;
        }

    }

}

