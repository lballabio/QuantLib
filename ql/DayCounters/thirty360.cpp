

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file thirty360.cpp
    \brief 30/360 day counters

    \fullpath
    ql/DayCounters/%thirty360.cpp
*/

// $Id$

#include <ql/DayCounters/thirty360.hpp>

namespace QuantLib {

    namespace DayCounters {

        Handle<DayCounter::DayCounterImpl>
        Thirty360::implementation(Thirty360::Convention c) {
            switch (c) {
              case USA:
                return Handle<DayCounterImpl>(new Thirty360USImpl);
              case European:
                return Handle<DayCounterImpl>(new Thirty360EuImpl);
              case Italian:
                return Handle<DayCounterImpl>(new Thirty360ItImpl);
              default:
                throw Error("Unknown 30/360 convention");
            }
        }


        int Thirty360::Thirty360USImpl::dayCount(
            const Date& d1, const Date& d2) const {
                int dd1 = d1.dayOfMonth(), dd2 = d2.dayOfMonth();
                int mm1 = d1.month(), mm2 = d2.month();
                int yy1 = d1.year(), yy2 = d2.year();

                if (dd2 == 31 && dd1 < 30) { dd2 = 1; mm2++; }

                return 360*(yy2-yy1) + 30*(mm2-mm1-1) +
                       QL_MAX(0,30-dd1) + QL_MIN(30,dd2);
        }

        int Thirty360::Thirty360EuImpl::dayCount(
            const Date& d1, const Date& d2) const {
                int dd1 = d1.dayOfMonth(), dd2 = d2.dayOfMonth();
                int mm1 = d1.month(), mm2 = d2.month();
                int yy1 = d1.year(), yy2 = d2.year();

                return 360*(yy2-yy1) + 30*(mm2-mm1-1) +
                       QL_MAX(0,30-dd1) + QL_MIN(30,dd2);
        }

        int Thirty360::Thirty360ItImpl::dayCount(
            const Date& d1, const Date& d2) const {
                int dd1 = d1.dayOfMonth(), dd2 = d2.dayOfMonth();
                int mm1 = d1.month(), mm2 = d2.month();
                int yy1 = d1.year(), yy2 = d2.year();

                if (mm1 == 2 && dd1 > 27) dd1 = 30;
                if (mm2 == 2 && dd2 > 27) dd2 = 30;

                return 360*(yy2-yy1) + 30*(mm2-mm1-1) +
                       QL_MAX(0,30-dd1) + QL_MIN(30,dd2);
        }

    }

}
