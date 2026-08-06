/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group
 Copyright (C) 2007, 2011 StatPro Italia srl

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

#include <ql/time/calendars/saudiarabia.hpp>
#include <ql/errors.hpp>
#include <algorithm>
#include <utility>
#include <vector>

namespace QuantLib {

    namespace {

        bool isTrueWeekend(Date d) {
            // The Saudi weekend was changed from 29th June 2013
            Weekday w = d.weekday();
            return (d < Date(29, June, 2013)) ?
                (w == Thursday || w == Friday) :
                (w == Friday || w == Saturday);
        }

        // In 2015 and 2014, the Eid holidays of the Tadawul Exchange
        // have been from Eid-1 to Eid+4
        // Sometimes, slightly longer holidays are observed 
        // But conservatively, we take Eid-1 to Eid+4 as the holiday

        // Eid Date    Holiday     Offset    Remarks
        // 2015-09-23    22-27     -1,+4     later extended to 22-28 or -1+5
        // 2015-07-17    18-21     -1,+4
        // 2014-10-05    03-11     -1,+4     because of weekend 03-11 is same as 04-09 
        // 2014-07-28    25-03     -1,+4     because of weekend 25-03 is same as 27-01
        // 2013-10-15    11-19     -2,+4     because of weekend 11-19 is same as 13-19 
        // 2013-08-08    06-12     -2,+4 
        // 2012-10-26    25-03     -1,+5     because of weekend 25-03 is same as 25-31 
        // 2012-08-19    16-24     -1,+4     because of weekend 16-24 is same as 18-23

        bool isEidAlAdha(Date d) {
            // Eid al Adha holiday windows for Tadawul.  Dates through 2022 use
            // Eid-1 to Eid+4 around the Gregorian Eid dates from
            // https://en.wikipedia.org/wiki/Eid_al-Adha#Eid_al-Adha_in_the_Gregorian_calendar
            // Windows from 2023 through 2029 are taken from the Saudi Exchange
            // holiday calendar at https://www.saudiexchange.sa
            static std::vector<std::pair<Date, Date>> EidAlAdha = {
                {Date(6, April, 1998), Date(11, April, 1998)},
                {Date(26, March, 1999), Date(31, March, 1999)},
                {Date(15, March, 2000), Date(20, March, 2000)},
                {Date(4, March, 2001), Date(9, March, 2001)},
                {Date(22, February, 2002), Date(27, February, 2002)},
                {Date(11, February, 2003), Date(16, February, 2003)},
                {Date(31, January, 2004), Date(5, February, 2004)},
                {Date(20, January, 2005), Date(25, January, 2005)},
                {Date(9, January, 2006), Date(14, January, 2006)},
                {Date(30, December, 2006), Date(4, January, 2007)},
                {Date(19, December, 2007), Date(24, December, 2007)},
                {Date(7, December, 2008), Date(12, December, 2008)},
                {Date(26, November, 2009), Date(1, December, 2009)},
                {Date(15, November, 2010), Date(20, November, 2010)},
                {Date(5, November, 2011), Date(10, November, 2011)},
                {Date(25, October, 2012), Date(30, October, 2012)},
                {Date(14, October, 2013), Date(19, October, 2013)},
                {Date(3, October, 2014), Date(8, October, 2014)},
                {Date(23, September, 2015), Date(28, September, 2015)},
                {Date(10, September, 2016), Date(15, September, 2016)},
                {Date(31, August, 2017), Date(5, September, 2017)},
                {Date(22, August, 2018), Date(27, August, 2018)},
                {Date(11, August, 2019), Date(16, August, 2019)},
                {Date(30, July, 2020), Date(4, August, 2020)},
                {Date(19, July, 2021), Date(24, July, 2021)},
                {Date(9, July, 2022), Date(14, July, 2022)},
                {Date(23, June, 2023), Date(1, July, 2023)},
                {Date(14, June, 2024), Date(22, June, 2024)},
                {Date(5, June, 2025), Date(10, June, 2025)},
                {Date(22, May, 2026), Date(30, May, 2026)},
                {Date(16, May, 2027), Date(20, May, 2027)},
                {Date(3, May, 2028), Date(9, May, 2028)},
                {Date(22, April, 2029), Date(26, April, 2029)},
            };

            return std::any_of(EidAlAdha.begin(), EidAlAdha.end(),
                               [=](const auto& p) { return d >= p.first && d <= p.second; });
        }

        bool isEidAlFitr(Date d) {
            // Eid al Fitr dates taken from:
            // https://en.wikipedia.org/wiki/Eid_al-Fitr#In_the_Gregorian_calendar
            static std::vector<Date> EidAlFitr = {
                Date(16, Dec, 2001),
                Date(5, Dec, 2002),
                Date(25, Nov, 2003),
                Date(13, Nov, 2004),
                Date(3, Nov, 2005),
                Date(23, Oct, 2006),
                Date(12, Oct, 2007),
                Date(30, Sep, 2008),
                Date(20, Sep, 2009),
                Date(10, Sep, 2010),
                Date(30, Aug, 2011),
                Date(19, Aug, 2012),
                Date(8, Aug, 2013),
                Date(28, Jul, 2014),
                Date(17, Jul, 2015),
                Date(6, Jul, 2016),
                Date(25, Jun, 2017),
                Date(15, Jun, 2018),
                Date(4, Jun, 2019),
                Date(24, May, 2020),
                Date(13, May, 2021),
                Date(2, May, 2022),
                Date(21, Apr, 2023),
                Date(10, Apr, 2024),
                Date(30, Mar, 2025),
                Date(20, Mar, 2026),
                Date(9, Mar, 2027),
                Date(26, Feb, 2028),
                Date(14, Feb, 2029)
            };

            return std::any_of(EidAlFitr.begin(), EidAlFitr.end(),
                               [=](Date p) { return d >= p - 1 && d <= p + 4; });
        }

    }

    SaudiArabia::SaudiArabia(Market market) {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> tadawulImpl(
                                                new SaudiArabia::TadawulImpl);
        switch (market) {
          case Tadawul:
            impl_ = tadawulImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }

    bool SaudiArabia::TadawulImpl::isWeekend(Weekday w) const {
        return w == Friday || w == Saturday;
    }

    bool SaudiArabia::TadawulImpl::isBusinessDay(const Date& date) const {
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        Weekday w = date.weekday();

        if (isTrueWeekend(date)
            || isEidAlAdha(date)
            || isEidAlFitr(date)
            // National Day
            || (d == 23 && m == September)
            // Founding Day (from 2022; rolls to the following Sunday when on Fri/Sat)
            || ((d == 22 || ((d == 23 || d == 24) && w == Sunday))
                && m == February && y >= 2022)
            // other one-shot holidays
            || (d == 26 && m == February && y==2011)
            || (d == 19 && m == March && y==2011)
            )
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

