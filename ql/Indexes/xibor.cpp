/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2004 StatPro Italia srl

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

#include <ql/Indexes/xibor.hpp>
#include <ql/Indexes/indexmanager.hpp>
#include <sstream>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED
    Xibor::Xibor(const std::string& familyName,
                 Integer n, TimeUnit units,
                 Integer settlementDays,
                 const Currency& currency,
                 const Calendar& calendar,
                 BusinessDayConvention convention,
                 const DayCounter& dayCounter,
                 const Handle<YieldTermStructure>& h)
    : familyName_(familyName), tenor_(n, units),
      settlementDays_(settlementDays),
      currency_(currency), calendar_(calendar),
      convention_(convention),
      dayCounter_(dayCounter), termStructure_(h) {
        registerWith(termStructure_);
        registerWith(Settings::instance().evaluationDate());
    }
    #endif
    Xibor::Xibor(const std::string& familyName,
                 const Period& tenor,
                 Integer settlementDays,
                 const Currency& currency,
                 const Calendar& calendar,
                 BusinessDayConvention convention,
                 const DayCounter& dayCounter,
                 const Handle<YieldTermStructure>& h)
    : familyName_(familyName), tenor_(tenor),
      settlementDays_(settlementDays),
      currency_(currency), calendar_(calendar),
      convention_(convention),
      dayCounter_(dayCounter), termStructure_(h) {
        registerWith(termStructure_);
        registerWith(Settings::instance().evaluationDate());
    }

    std::string Xibor::name() const {
        std::ostringstream out;
        out << familyName_ << io::short_period(tenor_)
            << " " << dayCounter_.name();
        return out.str();
    }

    Frequency Xibor::frequency() const {
        return tenor_.frequency();
    }

    Rate Xibor::fixing(const Date& fixingDate,
                       bool forecastTodaysFixing) const {
        Date today = Settings::instance().evaluationDate();
        if (fixingDate < today) {
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
        QL_REQUIRE(!termStructure_.empty(), "no term structure set");
        Date fixingValueDate = valueDate(fixingDate);
        Date endValueDate = maturityDate(fixingValueDate);
        DiscountFactor fixingDiscount =
            termStructure_->discount(fixingValueDate);
        DiscountFactor endDiscount =
            termStructure_->discount(endValueDate);
        Time fixingPeriod =
            dayCounter_.yearFraction(fixingValueDate, endValueDate);
        return (fixingDiscount/endDiscount-1.0) / fixingPeriod;
    }

    Date Xibor::valueDate(const Date& fixingDate) const {
        return calendar_.advance(fixingDate, settlementDays_, Days);
    }

    Date Xibor::maturityDate(const Date& valueDate) const {
        return calendar_.advance(valueDate, tenor_, convention_);
    }

}

