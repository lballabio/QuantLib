/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 StatPro Italia srl
 Copyright (C) 2015 Riccardo Barone
 Copyright (C) 2018 Matthias Lungwitz

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

#include <ql/time/calendars/romania.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Romania::Romania(Market market) {
        // all calendar instances share the same implementation instance
        static std::shared_ptr<Calendar::Impl> publicImpl =
            std::make_shared<Romania::PublicImpl>();
        static std::shared_ptr<Calendar::Impl> bvbImpl =
            std::make_shared<Romania::BVBImpl>();
        switch (market) {
          case Public:
            impl_ = publicImpl;
            break;
          case BVB:
            impl_ = bvbImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }

    bool Romania::PublicImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Day after New Year's Day
            || (d == 2 && m == January)
            // Unification Day
            || (d == 24 && m == January)
            // Orthodox Easter Monday
            || (dd == em)
            // Labour Day
            || (d == 1 && m == May)
            // Pentecost
            || (dd == em+49)
            // Children's Day (since 2017)
            || (d == 1 && m == June && y >= 2017)
            // St Marys Day
            || (d == 15 && m == August)
            // Feast of St Andrew
            || (d == 30 && m == November)
            // National Day
            || (d == 1 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // 2nd Day of Chritsmas
            || (d == 26 && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

    bool Romania::BVBImpl::isBusinessDay(const Date& date) const {
        if (!PublicImpl::isBusinessDay(date))
            return false;
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        if (// one-off closing days
            (d == 24 && m == December && y == 2014) ||
            (d == 31 && m == December && y == 2014)
            )
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }
            
}

