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

        // Tadawul Eid holiday windows documented below for 2012-2015; other
        // years through 2022 use Eid-1 to Eid+4 around published Gregorian
        // dates.  Windows from 2023 through 2029 follow the Saudi Exchange
        // holiday calendar at https://www.saudiexchange.sa
        //
        // Eid Date    Holiday     Offset    Remarks
        // 2015-09-23    22-27     -1,+4     Eid al-Adha
        // 2015-07-17    18-21     -1,+4     Eid al-Fitr
        // 2014-10-05    03-11     -1,+4     Eid al-Adha
        // 2014-07-28    25-03     -1,+4     Eid al-Fitr
        // 2013-10-15    11-19     -2,+4     Eid al-Adha
        // 2013-08-08    06-12     -2,+4     Eid al-Fitr
        // 2012-10-26    25-03     -1,+5     Eid al-Adha
        // 2012-08-19    16-24     -1,+4     Eid al-Fitr

        bool isEidAlAdha(Date d) {
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
                {Date(25, October, 2012), Date(3, November, 2012)},
                {Date(11, October, 2013), Date(19, October, 2013)},
                {Date(3, October, 2014), Date(11, October, 2014)},
                {Date(22, September, 2015), Date(27, September, 2015)},
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
            static std::vector<std::pair<Date, Date>> EidAlFitr = {
                {Date(15, December, 2001), Date(20, December, 2001)},
                {Date(4, December, 2002), Date(9, December, 2002)},
                {Date(24, November, 2003), Date(29, November, 2003)},
                {Date(12, November, 2004), Date(17, November, 2004)},
                {Date(2, November, 2005), Date(7, November, 2005)},
                {Date(22, October, 2006), Date(27, October, 2006)},
                {Date(11, October, 2007), Date(16, October, 2007)},
                {Date(29, September, 2008), Date(4, October, 2008)},
                {Date(19, September, 2009), Date(24, September, 2009)},
                {Date(9, September, 2010), Date(14, September, 2010)},
                {Date(29, August, 2011), Date(3, September, 2011)},
                {Date(16, August, 2012), Date(24, August, 2012)},
                {Date(6, August, 2013), Date(12, August, 2013)},
                {Date(25, July, 2014), Date(3, August, 2014)},
                {Date(18, July, 2015), Date(21, July, 2015)},
                {Date(5, July, 2016), Date(10, July, 2016)},
                {Date(24, June, 2017), Date(29, June, 2017)},
                {Date(14, June, 2018), Date(19, June, 2018)},
                {Date(3, June, 2019), Date(8, June, 2019)},
                {Date(23, May, 2020), Date(28, May, 2020)},
                {Date(12, May, 2021), Date(17, May, 2021)},
                {Date(1, May, 2022), Date(6, May, 2022)},
                {Date(17, April, 2023), Date(25, April, 2023)},
                {Date(4, April, 2024), Date(14, April, 2024)},
                {Date(27, March, 2025), Date(2, April, 2025)},
                {Date(17, March, 2026), Date(23, March, 2026)},
                {Date(7, March, 2027), Date(11, March, 2027)},
                {Date(27, February, 2028), Date(2, March, 2028)},
                {Date(12, February, 2029), Date(18, February, 2029)},
            };

            return std::any_of(EidAlFitr.begin(), EidAlFitr.end(),
                               [=](const auto& p) { return d >= p.first && d <= p.second; });
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

