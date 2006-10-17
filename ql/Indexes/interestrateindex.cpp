/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Indexes/interestrateindex.hpp>
#include <ql/Indexes/indexmanager.hpp>
#include <sstream>

namespace QuantLib {

    InterestRateIndex::InterestRateIndex(const std::string& familyName,
                                         const Period& tenor,
                                         Integer settlementDays,
                                         const Currency& currency,
                                         const Calendar& calendar,
                                         const DayCounter& dayCounter)
    : familyName_(familyName), tenor_(tenor), settlementDays_(settlementDays),
      currency_(currency), calendar_(calendar), dayCounter_(dayCounter)
    {
        registerWith(Settings::instance().evaluationDate());
        registerWith(IndexManager::instance().notifier(name()));
    }

    std::string InterestRateIndex::name() const {
        std::ostringstream out;
        out << familyName_ << io::short_period(tenor_)
            << " " << dayCounter_.name();
        return out.str();
    }

    bool InterestRateIndex::isValidFixingDate(const Date& fixingDate) const {
        return calendar_.isBusinessDay(fixingDate);
    }

    Rate InterestRateIndex::fixing(const Date& fixingDate,
                                   bool forecastTodaysFixing) const {
        QL_REQUIRE(isValidFixingDate(fixingDate),
                   "Fixing date " << fixingDate << " is not valid");
        Date today = Settings::instance().evaluationDate();
        bool enforceTodaysHistoricFixings = 
            Settings::instance().enforceTodaysHistoricFixings();
        if (fixingDate < today ||
            (fixingDate == today) && enforceTodaysHistoricFixings) {
            // must have been fixed
            Rate pastFixing =
                IndexManager::instance().getHistory(name())[fixingDate];
            QL_REQUIRE(pastFixing != Null<Real>(),
                       "Missing " << name() << " fixing for " << fixingDate);
            return pastFixing;
        }
        if ((fixingDate == today) && !forecastTodaysFixing) {
            // might have been fixed
            try {
                Rate pastFixing =
                    IndexManager::instance().getHistory(name())[fixingDate];
                if (pastFixing != Null<Real>())
                    return pastFixing;
                else
                    ;   // fall through and forecast
            } catch (Error&) {
                ;       // fall through and forecast
            }
        }
        // forecast
        return forecastFixing(fixingDate);
    }

    Date InterestRateIndex::valueDate(const Date& fixingDate) const {
        QL_REQUIRE(isValidFixingDate(fixingDate),
                   "Fixing date " << fixingDate << " is not valid");
        return calendar_.advance(fixingDate, settlementDays_, Days);
    }

    #ifndef QL_DISABLE_DEPRECATED
    Date InterestRateIndex::maturityDate(const Date& valueDate) const {
        return calendar_.advance(valueDate, tenor_, Unadjusted);
    }
    #endif

}

