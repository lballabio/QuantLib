/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include <ql/experimental/fx/fxindex.hpp>
#include <ql/currencies/exchangeratemanager.hpp>

namespace QuantLib {

FxIndex::FxIndex(const std::string &familyName, Natural fixingDays,
                 const Currency &source, const Currency &target,
                 const Calendar &fixingCalendar,
                 const Handle<YieldTermStructure> &sourceYts,
                 const Handle<YieldTermStructure> &targetYts)
    : familyName_(familyName), fixingDays_(fixingDays), sourceCurrency_(source),
      targetCurrency_(target), sourceYts_(sourceYts), targetYts_(targetYts),
      fixingCalendar_(fixingCalendar) {

    std::ostringstream tmp;
    tmp << familyName_ << " " << sourceCurrency_.name() << "/"
        << targetCurrency_.name();
    name_ = tmp.str();

    registerWith(Settings::instance().evaluationDate());
    registerWith(IndexManager::instance().notifier(name()));

    // we should register with the exchange rate manager
    // to be notified of changes in the spot exchange rate
    // however currently exchange rates are not quotes anyway
    // so this is to be revisited later
}

Real FxIndex::fixing(const Date &fixingDate, bool forecastTodaysFixing) const {

    QL_REQUIRE(isValidFixingDate(fixingDate),
               "Fixing date " << fixingDate << " is not valid");

    Date today = Settings::instance().evaluationDate();

    if (fixingDate > today || (fixingDate == today && forecastTodaysFixing))
        return forecastFixing(fixingDate);

    Real result = Null<Decimal>();

    if (fixingDate < today ||
        Settings::instance().enforcesTodaysHistoricFixings()) {
        // must have been fixed
        // do not catch exceptions
        result = pastFixing(fixingDate);
        QL_REQUIRE(result != Null<Real>(),
                   "Missing " << name() << " fixing for " << fixingDate);
    } else {
        try {
            // might have been fixed
            result = pastFixing(fixingDate);
        } catch (Error &) {
            ; // fall through and forecast
        }
        if (result == Null<Real>())
            return forecastFixing(fixingDate);
    }

    return result;
}

Real FxIndex::forecastFixing(const Date &fixingDate) const {

    QL_REQUIRE(!sourceYts_.empty() && !targetYts_.empty(),
               "null term structure set to this instance of " << name());

    // we base the forecast always on the exchange rate (and not on today's
    // fixing)
    Real rate = ExchangeRateManager::instance()
                    .lookup(sourceCurrency_, targetCurrency_)
                    .rate();

    // the exchange rate is interpreted as the spot rate w.r.t. the index's
    // settlement date
    Date refValueDate = valueDate(Settings::instance().evaluationDate());

    // the fixing is obeying the settlement delay as well
    Date fixingValueDate = valueDate(fixingDate);

    // we can assume fixingValueDate >= valueDate
    QL_REQUIRE(
        fixingValueDate >= refValueDate,
        "value date for requested fixing as of "
            << fixingDate << " (" << fixingValueDate
            << ") must be greater or equal to today's fixing value date ("
            << refValueDate << ")");

    // compute the forecast applying the usual no arbitrage principle
    Real forward = rate * sourceYts_->discount(fixingValueDate) *
                   targetYts_->discount(refValueDate) /
                   (sourceYts_->discount(refValueDate) *
                    targetYts_->discount(fixingValueDate));

    return forward;
}

} // namespace QuantLib
