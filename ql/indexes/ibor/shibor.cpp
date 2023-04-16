/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Cheng Li

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

#include <ql/indexes/ibor/shibor.hpp>
#include <ql/currencies/asia.hpp>
#include <ql/time/calendars/china.hpp>
#include <ql/time/daycounters/actual360.hpp>

namespace QuantLib {

    namespace {

        BusinessDayConvention shiborConvention(const Period& p) {
            switch (p.units()) {
              case Days:
              case Weeks:
                return Following;
              case Months:
              case Years:
                return ModifiedFollowing;
              default:
                QL_FAIL("invalid time units");
            }
        }

    }

    Shibor::Shibor(const Period& tenor,
                   const Handle<YieldTermStructure>& h)
    : IborIndex("Shibor", tenor, (tenor == 1*Days? 0 : 1), CNYCurrency(),
                China(China::IB), shiborConvention(tenor), false,
                Actual360(), h) {}
    
    std::shared_ptr<IborIndex> Shibor::clone(
                                  const Handle<YieldTermStructure>& h) const {
        return std::shared_ptr<IborIndex>(new Shibor(tenor(), h));
    }
}
