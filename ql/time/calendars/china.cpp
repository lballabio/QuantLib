/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group
 Copyright (C) 2007, 2009, 2010, 2011 StatPro Italia srl

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

#include <ql/time/calendars/china.hpp>
#include <ql/errors.hpp>
#include <set>

namespace QuantLib {

    China::China(Market m) {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> sseImpl(new China::SseImpl);
        static boost::shared_ptr<Calendar::Impl> IBImpl(new China::IbImpl);
        switch (m) {
          case SSE:
            impl_ = sseImpl;
            break;
          case IB:
            impl_ = IBImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }

    bool China::SseImpl::isWeekend(Weekday w) const {
        return w == Saturday || w == Sunday;
    }

    bool China::SseImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();

        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            || (y == 2005 && d == 3 && m == January)
            || (y == 2006 && (d == 2 || d == 3) && m == January)
            || (y == 2007 && d <= 3 && m == January)
            || (y == 2007 && d == 31 && m == December)
            || (y == 2009 && d == 2 && m == January)
            || (y == 2011 && d == 3 && m == January)
            || (y == 2012 && (d == 2 || d == 3) && m == January)
            || (y == 2013 && d <= 3 && m == January)
            || (y == 2014 && d == 1 && m == January)
            || (y == 2015 && d <= 3 && m == January)
            // Chinese New Year
            || (y == 2004 && d >= 19 && d <= 28 && m == January)
            || (y == 2005 && d >=  7 && d <= 15 && m == February)
            || (y == 2006 && ((d >= 26 && m == January) ||
                              (d <= 3 && m == February)))
            || (y == 2007 && d >= 17 && d <= 25 && m == February)
            || (y == 2008 && d >= 6 && d <= 12 && m == February)
            || (y == 2009 && d >= 26 && d <= 30 && m == January)
            || (y == 2010 && d >= 15 && d <= 19 && m == February)
            || (y == 2011 && d >= 2 && d <= 8 && m == February)
            || (y == 2012 && d >= 23 && d <= 28 && m == January)
            || (y == 2013 && d >= 11 && d <= 15 && m == February)
            || (y == 2014 && d >= 31 && m == January)
            || (y == 2014 && d <= 6 && m == February)
            || (y == 2015 && d >= 18 && d <= 24 && m == February)
            // Ching Ming Festival
            || (y <= 2008 && d == 4 && m == April)
            || (y == 2009 && d == 6 && m == April)
            || (y == 2010 && d == 5 && m == April)
            || (y == 2011 && d >=3 && d <= 5 && m == April)
            || (y == 2012 && d >= 2 && d <= 4 && m == April)
            || (y == 2013 && d >= 4 && d <= 5 && m == April)
            || (y == 2014 && d == 7 && m == April)
            || (y == 2015 && d >= 5 && d <= 6 && m == April)
            // Labor Day
            || (y <= 2007 && d >= 1 && d <= 7 && m == May)
            || (y == 2008 && d >= 1 && d <= 2 && m == May)
            || (y == 2009 && d == 1 && m == May)
            || (y == 2010 && d == 3 && m == May)
            || (y == 2011 && d == 2 && m == May)
            || (y == 2012 && ((d == 30 && m == April) ||
                              (d == 1 && m == May)))
            || (y == 2013 && ((d >= 29 && m == April) ||
                              (d == 1 && m == May)))
            || (y == 2014 && d >= 1 && d <=3 && m == May)
            || (y == 2015 && d == 1 && m == May)
            // Tuen Ng Festival
            || (y <= 2008 && d == 9 && m == June)
            || (y == 2009 && (d == 28 || d == 29) && m == May)
            || (y == 2010 && d >= 14 && d <= 16 && m == June)
            || (y == 2011 && d >= 4 && d <= 6 && m == June)
            || (y == 2012 && d >= 22 && d <= 24 && m == June)
            || (y == 2013 && d >= 10 && d <= 12 && m == June)
            || (y == 2014 && d == 2 && m == June)
            || (y == 2015 && d == 22 && m == June)
            // Mid-Autumn Festival
            || (y <= 2008 && d == 15 && m == September)
            || (y == 2010 && d >= 22 && d <= 24 && m == September)
            || (y == 2011 && d >= 10 && d <= 12 && m == September)
            || (y == 2012 && d == 30 && m == September)
            || (y == 2013 && d >= 19 && d <= 20 && m == September)
            || (y == 2014 && d == 8 && m == September)
            || (y == 2015 && d == 27 && m == September)
            // National Day
            || (y <= 2007 && d >= 1 && d <= 7 && m == October)
            || (y == 2008 && ((d >= 29 && m == September) ||
                              (d <= 3 && m == October)))
            || (y == 2009 && d >= 1 && d <= 8 && m == October)
            || (y == 2010 && d >= 1 && d <= 7 && m == October)
            || (y == 2011 && d >= 1 && d <= 7 && m == October)
            || (y == 2012 && d >= 1 && d <= 7 && m == October)
            || (y == 2013 && d >= 1 && d <= 7 && m == October)
            || (y == 2014 && d >= 1 && d <= 7 && m == October)
            || (y == 2015 && d >= 1 && d <= 7 && m == October)
            // 70th anniversary of the victory of anti-Japaneses war
            || (y == 2015 && d >= 3 && d <= 4 && m == September)
            )
            return false;
        return true;
    }

    bool China::IbImpl::isWeekend(Weekday w) const {
        return w == Saturday || w == Sunday;
    }

    bool China::IbImpl::isBusinessDay(const Date& date) const {
        static const Date working_weekends[] = {
            // 2005
            Date(5, February, 2005),
            Date(6, February, 2005),
            Date(30, April, 2005),
            Date(8, May, 2005),
            Date(8, October, 2005),
            Date(9, October, 2005),
            Date(31, December, 2005),
            //2006
            Date(28, January, 2006),
            Date(29, April, 2006),
            Date(30, April, 2006),
            Date(30, September, 2006),
            Date(30, December, 2006),
            Date(31, December, 2006),
            // 2007
            Date(17, February, 2007),
            Date(25, February, 2007),
            Date(28, April, 2007),
            Date(29, April, 2007),
            Date(29, September, 2007),
            Date(30, September, 2007),
            Date(29, December, 2007),
            // 2008
            Date(2, February, 2008),
            Date(3, February, 2008),
            Date(4, May, 2008),
            Date(27, September, 2008),
            Date(28, September, 2008),
            // 2009
            Date(4, January, 2009),
            Date(24, January, 2009),
            Date(1, February, 2009),
            Date(31, May, 2009),
            Date(27, September, 2009),
            Date(10, October, 2009),
            // 2010
            Date(20, February, 2010),
            Date(21, February, 2010),
            Date(12, June, 2010),
            Date(13, June, 2010),
            Date(19, September, 2010),
            Date(25, September, 2010),
            Date(26, September, 2010),
            Date(9, October, 2010),
            // 2011
            Date(30, January, 2011),
            Date(12, February, 2011),
            Date(2, April, 2011),
            Date(8, October, 2011),
            Date(9, October, 2011),
            Date(31, December, 2011),
            // 2012
            Date(21, January, 2012),
            Date(29, January, 2012),
            Date(31, March, 2012),
            Date(1, April, 2012),
            Date(28, April, 2012),
            Date(29, September, 2012),
            // 2013
            Date(5,January,2013),
            Date(6,January,2013),
            Date(16,February,2013),
            Date(17,February,2013),
            Date(7,April,2013),
            Date(27,April,2013),
            Date(28,April,2013),
            Date(8,June,2013),
            Date(9,June,2013),
            Date(22,September,2013),
            Date(29,September,2013),
            Date(12,October,2013),
            // 2014
            Date(26,January,2014),
            Date(8,February,2014),
            Date(4,May,2014),
            Date(28,September,2014),
            Date(11,October,2014),
            // 2015
            Date(4,January,2015),
            Date(15,February,2015),
            Date(28,February,2015),
            Date(6,September,2015),
            Date(10,October,2015)
        };
        static const Size n =
            sizeof(working_weekends)/sizeof(working_weekends[0]);
        static const std::set<Date> workingWeekends(working_weekends+0,
                                                    working_weekends+n);

        // If it is already a SSE business day, it must be a IB business day
        return sseImpl->isBusinessDay(date) ||
            (workingWeekends.find(date) != workingWeekends.end());
    }

}

