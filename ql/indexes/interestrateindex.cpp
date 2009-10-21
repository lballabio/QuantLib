/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/indexes/interestrateindex.hpp>
#include <ql/settings.hpp>

#include <sstream>

namespace QuantLib {

    InterestRateIndex::InterestRateIndex(const std::string& familyName,
                                         const Period& tenor,
                                         Natural fixingDays,
                                         const Currency& currency,
                                         const Calendar& fixingCalendar,
                                         const DayCounter& dayCounter)
    : familyName_(familyName), tenor_(tenor), fixingDays_(fixingDays),
      fixingCalendar_(fixingCalendar), currency_(currency),
      dayCounter_(dayCounter) {
        tenor_.normalize();
        registerWith(Settings::instance().evaluationDate());
        registerWith(IndexManager::instance().notifier(name()));
    }

    std::string InterestRateIndex::name() const {
        std::ostringstream out;
        out << familyName_;
        if (tenor_ == 1*Days) {
            if (fixingDays_==0)
                out << "ON";
            else if (fixingDays_==1)
                out << "TN";
            else if (fixingDays_==2)
                out << "SN";
            else
                out << io::short_period(tenor_);
        } else {
            out << io::short_period(tenor_);
        }
        out << " " << dayCounter_.name();
        return out.str();
    }

    Calendar InterestRateIndex::fixingCalendar() const {
        return fixingCalendar_;
    }

    bool InterestRateIndex::isValidFixingDate(const Date& fixingDate) const {
        return fixingCalendar().isBusinessDay(fixingDate);
    }

    Rate InterestRateIndex::fixing(const Date& fixingDate,
                                   bool forecastTodaysFixing) const {
        QL_REQUIRE(isValidFixingDate(fixingDate),
                   "Fixing date " << fixingDate << " is not valid");
        Date today = Settings::instance().evaluationDate();
        bool enforceTodaysHistoricFixings =
            Settings::instance().enforcesTodaysHistoricFixings();
        if (fixingDate < today ||
            (fixingDate == today && enforceTodaysHistoricFixings && !forecastTodaysFixing)) {
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
        return fixingCalendar().advance(fixingDate, fixingDays_, Days);
    }

    Date InterestRateIndex::fixingDate(const Date& valueDate) const {
        Date fixingDate = fixingCalendar().advance(valueDate,
            -static_cast<Integer>(fixingDays_), Days);
        QL_ENSURE(isValidFixingDate(fixingDate),
                  "fixing date " << fixingDate << " is not valid");
        return fixingDate;
    }

}
