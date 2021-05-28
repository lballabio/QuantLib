/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2018 Alexey Indiryakov

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

#include <ql/time/daycounters/thirty360.hpp>
#include <algorithm>

namespace QuantLib {

    ext::shared_ptr<DayCounter::Impl>
    Thirty360::implementation(Thirty360::Convention c, const Date& terminationDate, bool isLastPeriod) {
        switch (c) {
          case USA:
          case NASD:
            return ext::shared_ptr<DayCounter::Impl>(new US_Impl);
          case European:
          case EurobondBasis:
            return ext::shared_ptr<DayCounter::Impl>(new EU_Impl);
          case Italian:
            return ext::shared_ptr<DayCounter::Impl>(new IT_Impl);
          case ISMA:
          case BondBasis:
            return ext::shared_ptr<DayCounter::Impl>(new ISMA_Impl);
          case ISDA:
          case German:
            return ext::shared_ptr<DayCounter::Impl>(new ISDA_Impl(terminationDate, isLastPeriod));
          default:
            QL_FAIL("unknown 30/360 convention");
        }
    }

    Date::serial_type Thirty360::US_Impl::dayCount(const Date& d1,
                                                   const Date& d2) const {
        Day dd1 = d1.dayOfMonth(), dd2 = d2.dayOfMonth();
        Integer mm1 = d1.month(), mm2 = d2.month();
        Year yy1 = d1.year(), yy2 = d2.year();

        if (dd1 == 31) { dd1 = 30; }
        if (dd2 == 31 && dd1 >= 30) { dd2 = 30; }
        if (dd2 == 31 && dd1 < 30) { dd2 = 1; mm2++; }

        return 360*(yy2-yy1) + 30*(mm2-mm1) + (dd2-dd1);
    }

    Date::serial_type Thirty360::ISMA_Impl::dayCount(const Date& d1,
                                                     const Date& d2) const {
        Day dd1 = d1.dayOfMonth(), dd2 = d2.dayOfMonth();
        Integer mm1 = d1.month(), mm2 = d2.month();
        Year yy1 = d1.year(), yy2 = d2.year();

        if (dd1 == 31) { dd1 = 30; }
        if (dd2 == 31 && dd1 == 30) { dd2 = 30; }

        return 360*(yy2-yy1) + 30*(mm2-mm1) + (dd2-dd1);
    }

    Date::serial_type Thirty360::EU_Impl::dayCount(const Date& d1,
                                                   const Date& d2) const {
        Day dd1 = d1.dayOfMonth(), dd2 = d2.dayOfMonth();
        Month mm1 = d1.month(), mm2 = d2.month();
        Year yy1 = d1.year(), yy2 = d2.year();

        if (dd1 == 31) { dd1 = 30; }
        if (dd2 == 31) { dd2 = 30; }

        return 360*(yy2-yy1) + 30*(mm2-mm1) + (dd2-dd1);
    }

    Date::serial_type Thirty360::IT_Impl::dayCount(const Date& d1,
                                                   const Date& d2) const {
        Day dd1 = d1.dayOfMonth(), dd2 = d2.dayOfMonth();
        Month mm1 = d1.month(), mm2 = d2.month();
        Year yy1 = d1.year(), yy2 = d2.year();

        if (dd1 == 31) { dd1 = 30; }
        if (dd2 == 31) { dd2 = 30; }

        if (mm1 == 2 && dd1 > 27) { dd1 = 30; }
        if (mm2 == 2 && dd2 > 27) { dd2 = 30; }

        return 360*(yy2-yy1) + 30*(mm2-mm1) + (dd2-dd1);
    }

    Date::serial_type Thirty360::ISDA_Impl::dayCount(const Date& d1,
                                                     const Date& d2) const {
        Day dd1 = d1.dayOfMonth(), dd2 = d2.dayOfMonth();
        Month mm1 = d1.month(), mm2 = d2.month();
        Year yy1 = d1.year(), yy2 = d2.year();

        if (dd1 == 31) { dd1 = 30; }
        if (dd2 == 31) { dd2 = 30; }

        if (mm1 == 2 && dd1 == 28 + (Date::isLeap(yy1) ? 1 : 0)) { dd1 = 30; }

        bool isTerminationDate =
            terminationDate_ == Date() ? isLastPeriod_ : d2 == terminationDate_;
        if (!isTerminationDate && mm2 == 2 && dd2 == 28 + (Date::isLeap(yy2) ? 1 : 0)) { dd2 = 30; }

        return 360*(yy2-yy1) + 30*(mm2-mm1) + (dd2-dd1);
    }

}
