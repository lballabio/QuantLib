/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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

#include <ql/index.hpp>

namespace QuantLib {

    void Index::addFixing(const Date& fixingDate, Real fixing) {
        QL_REQUIRE(isValidFixingDate(fixingDate),
                   "Fixing date " << fixingDate.weekday() << ", " <<
                   fixingDate << " is not valid");
        std::string tag = name();
        TimeSeries<Real> h = IndexManager::instance().getHistory(tag);
        h[fixingDate] = fixing;
        IndexManager::instance().setHistory(tag,h);
    }

    void Index::clearFixings() {
        IndexManager::instance().clearHistory(name());
    }

    bool Index::isValidFixingDate(const Date& fixingDate) const {
        return fixingCalendar().isBusinessDay(fixingDate);
    }

}
