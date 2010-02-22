/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 StatPro Italia srl

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

#include <ql/index.hpp>

namespace QuantLib {

    void Index::addFixing(const Date& fixingDate,
                          Real fixing,
                          bool forceOverwrite) {
        addFixings(&fixingDate, (&fixingDate)+1,
                   &fixing,
                   forceOverwrite);
    }

    void Index::addFixings(const TimeSeries<Real>& t,
                           bool forceOverwrite) {
        // is there a way of iterating over dates and values
        // without having to make a copy?
        std::vector<Date> dates = t.dates();
        std::vector<Real> values = t.values();
        addFixings(dates.begin(), dates.end(),
                   values.begin(),
                   forceOverwrite);
    }

    void Index::clearFixings() {
        IndexManager::instance().clearHistory(name());
    }

}
