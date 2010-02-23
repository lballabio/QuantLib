/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Piter Dias

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

/*! \file business252.hpp
    \brief business/252 day counter
*/

#ifndef quantlib_business252_day_counter_hpp
#define quantlib_business252_day_counter_hpp

#include <ql/time/daycounter.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/calendars/brazil.hpp>

namespace QuantLib {

    //! Business/252 day count convention
    /*! \ingroup daycounters */
    class Business252 : public DayCounter {
      private:
        class Impl : public DayCounter::Impl {
          private:
            Calendar calendar_;
          public:
            std::string name() const {
                std::ostringstream out;
                out << "Business/252(" << calendar_.name() << ")";
                return out.str();
            }
            BigInteger dayCount(const Date& d1,
                                const Date& d2) const {
                return calendar_.businessDaysBetween(d1, d2);
            }
            Time yearFraction(const Date& d1,
                              const Date& d2,
                              const Date&,
                              const Date&) const {
                return dayCount(d1, d2)/252.0;
            }
            Impl(Calendar c) { calendar_ = c; }
        };
      public:
        Business252(Calendar c = Brazil())
        : DayCounter(boost::shared_ptr<DayCounter::Impl>(
                                                 new Business252::Impl(c))) {}
    };

}

#endif
