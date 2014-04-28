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
            // Chinese New Year
            || (y == 2004 && d >= 19 && d <= 28 && m == January)
            || (y == 2005 && d >=  7 && d <= 15 && m == February)
            || (y == 2006 && ((d >= 26 && m == January) ||
                              (d <= 3 && m == February)))
            || (y == 2007 && d >= 17 && d <= 25 && m == February)
            || (y == 2008 && d >= 6 && d <= 12 && m == February)
            || (y == 2009 && d >= 26 && d <= 30 && m == January)
            || (y == 2010 && d >= 15 && d <= 19 && m == January)
            || (y == 2011 && d >= 2 && d <= 8 && m == February)
            || (y == 2012 && d >= 23 && d <= 28 && m == January)
            || (y == 2013 && d >= 11 && d <= 15 && m == February)
            || (y == 2014 && d >= 31 && m == January)
            || (y == 2014 && d <= 6 && m == February)
            // Ching Ming Festival
            || (y <= 2008 && d == 4 && m == April)
            || (y == 2009 && d == 6 && m == April)
            || (y == 2010 && d == 5 && m == April)
            || (y == 2011 && d >=3 && d <= 5 && m == April)
            || (y == 2012 && d >= 2 && d <= 4 && m == April)
            || (y == 2013 && d >= 4 && d <= 5 && m == April)
            || (y == 2014 && d == 7 && m == April)
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
            // Tuen Ng Festival
            || (y <= 2008 && d == 9 && m == June)
            || (y == 2009 && (d == 28 || d == 29) && m == May)
            || (y == 2010 && d >= 14 && d <= 16 && m == June)
            || (y == 2011 && d >= 4 && d <= 6 && m == June)
            || (y == 2012 && d >= 22 && d <= 24 && m == June)
            || (y == 2013 && d >= 10 && d <= 12 && m == June)
            || (y == 2014 && d == 2 && m == June)
            // Mid-Autumn Festival
            || (y <= 2008 && d == 15 && m == September)
            || (y == 2010 && d >= 22 && d <= 24 && m == September)
            || (y == 2011 && d >= 10 && d <= 12 && m == September)
            || (y == 2012 && d == 30 && m == September)
            || (y == 2013 && d >= 19 && d <= 20 && m == September)
            || (y == 2014 && d == 8 && m == September)
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
            )
            return false;
        return true;
    }

    bool China::IbImpl::isWeekend(Weekday w) const {
        return w == Saturday || w == Sunday;
    }

    bool China::IbImpl::isBusinessDay(const Date& date) const {

        bool isNormalBizDay = sseImpl->isBusinessDay(date);

        if(isNormalBizDay) {
            // If it is already a SSE business day, it must be a IB business day
            return isNormalBizDay;
        } else {
            const std::set<Date>& badWeekendList = badWeekends();
            if(badWeekendList.find(date) != badWeekendList.end())
                return true;
            else
                return false;
        }
    }

    const std::set<Date>& China::IbImpl::badWeekends() const {

        // Hard coded working weekends

        static const std::vector<Date> vec
            = boost::assign::list_of<Date>
            // 2013
            (Date(5,Jan,2013))(Date(6,Jan,2013))(Date(16,Feb,2013))(Date(17,Feb,2013))(Date(7,Apr,2013))(Date(27,Apr,2013))(Date(28,Apr,2013))
            (Date(8,Jun,2013))(Date(9,Jun,2013))(Date(22,Sep,2013))(Date(29,Sep,2013))(Date(12,Oct,2013))
            // 2014
            (Date(26,Jan,2014))(Date(8,Feb,2014))(Date(4,May,2014))(Date(28,Sep,2014))(Date(11,Oct,2014));

        static const std::set<Date> badWeekendList(vec.begin(), vec.end());
        return badWeekendList;
    }

}

