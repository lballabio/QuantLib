/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group
 Copyright (C) 2008, 2009, 2010, 2011 StatPro Italia srl

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

/*! \file china.hpp
    \brief Chinese calendar
*/

#ifndef quantlib_chinese_calendar_hpp
#define quantlib_chinese_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Chinese calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's day, January 1st (possibly followed by one or
            two more holidays)</li>
        <li>Labour Day, first week in May</li>
        <li>National Day, one week from October 1st</li>
        </ul>

        Other holidays for which no rule is given (data available for
        2004-2019 only):
        <ul>
        <li>Chinese New Year</li>
        <li>Ching Ming Festival</li>
        <li>Tuen Ng Festival</li>
        <li>Mid-Autumn Festival</li>
        <li>70th anniversary of the victory of anti-Japaneses war</li>
        </ul>

        SSE data from <http://www.sse.com.cn/>
        IB data from <http://www.chinamoney.com.cn/>

        \ingroup calendars
    */
    class China : public Calendar {
      private:
        class SseImpl : public Calendar::Impl {
          public:
            std::string name() const override { return "Shanghai stock exchange"; }
            bool isWeekend(Weekday) const override;
            bool isBusinessDay(const Date&) const override;
        };

        class IbImpl : public Calendar::Impl {
          public:
            IbImpl()
            : sseImpl(ext::make_shared<China::SseImpl>()) {}
            std::string name() const override { return "China inter bank market"; }
            bool isWeekend(Weekday) const override;
            bool isBusinessDay(const Date&) const override;

          private:
            ext::shared_ptr<Calendar::Impl> sseImpl;
        };
      public:
        enum Market { SSE,    //!< Shanghai stock exchange
                      IB      //!< Interbank calendar
        };
        China(Market m = SSE);
    };

}


#endif


#ifndef id_be36760d543fc520d5b26e54d1585a09
#define id_be36760d543fc520d5b26e54d1585a09
inline bool test_be36760d543fc520d5b26e54d1585a09(int* i) { return i != 0; }
#endif
