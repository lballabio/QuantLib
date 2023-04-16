/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Ferdinando Ametrano

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

#include <ql/experimental/coupons/proxyibor.hpp>
#include <utility>

namespace QuantLib {

    ProxyIbor::ProxyIbor(const std::string& familyName,
                         const Period& tenor,
                         Natural settlementDays,
                         const Currency& currency,
                         const Calendar& fixingCalendar,
                         BusinessDayConvention convention,
                         bool endOfMonth,
                         const DayCounter& dayCounter,
                         Handle<Quote> gearing,
                         std::shared_ptr<IborIndex> iborIndex,
                         Handle<Quote> spread)
    : IborIndex(familyName,
                tenor,
                settlementDays,
                currency,
                fixingCalendar,
                convention,
                endOfMonth,
                dayCounter),
      gearing_(std::move(gearing)), iborIndex_(std::move(iborIndex)), spread_(std::move(spread)) {
        registerWith(iborIndex_);
    }
}
