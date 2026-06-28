/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/indexes/fxindex.hpp>
#include <ql/settings.hpp>
#include <sstream>
#include <utility>

namespace QuantLib {

FxIndex::FxIndex(std::string familyName,
                 Currency sourceCurrency,
                 Currency targetCurrency,
                 Calendar fixingCalendar)
: familyName_(std::move(familyName)), sourceCurrency_(std::move(sourceCurrency)),
  targetCurrency_(std::move(targetCurrency)), fixingCalendar_(std::move(fixingCalendar)) {

    std::ostringstream out;
    out << familyName_ << " " << sourceCurrency_.code() << "/" << targetCurrency_.code();
    name_ = out.str();

    registerWith(Settings::instance().evaluationDate());
    registerWith(notifier());
}

std::string FxIndex::name() const { return name_; }

Calendar FxIndex::fixingCalendar() const { return fixingCalendar_; }

bool FxIndex::isValidFixingDate(const Date& fixingDate) const {
    return fixingCalendar_.isBusinessDay(fixingDate);
}

Real FxIndex::fixing(const Date& fixingDate, bool) const {

    QL_REQUIRE(isValidFixingDate(fixingDate), "Fixing date " << fixingDate << " is not valid");

    Date today = Settings::instance().evaluationDate();
    QL_REQUIRE(fixingDate <= today,
               name() << " stores historical fixings only and cannot forecast the fixing for "
                      << fixingDate
                      << "; future FX resets are projected by the pricing engine from the "
                         "discount curves");

    Real result = pastFixing(fixingDate);
    QL_REQUIRE(result != Null<Real>(), "Missing " << name() << " fixing for " << fixingDate);
    return result;
}

} // namespace QuantLib
