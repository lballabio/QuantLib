/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2011 Ferdinando Ametrano
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
#include <utility>

namespace QuantLib {

    InterestRateIndex::InterestRateIndex(std::string familyName,
                                         const Period& tenor,
                                         Natural fixingDays,
                                         Currency currency,
                                         Calendar fixingCalendar,
                                         DayCounter dayCounter)
    : familyName_(std::move(familyName)), tenor_(tenor), fixingDays_(fixingDays),
      currency_(std::move(currency)), dayCounter_(std::move(dayCounter)),
      fixingCalendar_(std::move(fixingCalendar)) {
        // tenor_.normalize() does too much; we want to leave days alone
        if (tenor.units() == Months && tenor.length() % 12 == 0)
            tenor_ = Period(tenor.length() / 12, Years);

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
        name_ = out.str();

        registerWith(Settings::instance().evaluationDate());
        registerWith(IndexManager::instance().notifier(InterestRateIndex::name()));
    }

    Rate InterestRateIndex::fixing(const Date& fixingDate,
                                   bool forecastTodaysFixing) const {

        QL_REQUIRE(isValidFixingDate(fixingDate),
                   "Fixing date " << fixingDate << " is not valid");

        Date today = Settings::instance().evaluationDate();

        if (fixingDate>today ||
            (fixingDate==today && forecastTodaysFixing))
            return forecastFixing(fixingDate);

        if (fixingDate<today ||
            Settings::instance().enforcesTodaysHistoricFixings()) {
            // must have been fixed
            // do not catch exceptions
            Rate result = pastFixing(fixingDate);
            QL_REQUIRE(result != Null<Real>(),
                       "Missing " << name() << " fixing for " << fixingDate);
            return result;
        }

        try {
            // might have been fixed
            Rate result = pastFixing(fixingDate);
            if (result!=Null<Real>())
                return result;
            else
                ;   // fall through and forecast
        } catch (Error&) {
                ;   // fall through and forecast
        }
        return forecastFixing(fixingDate);
    }

}
