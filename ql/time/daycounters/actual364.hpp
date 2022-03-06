/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 StatPro Italia srl

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

/*! \file actual364.hpp
    \brief Actual/364 day counter
*/

#ifndef quantlib_actual364_day_counter_hpp
#define quantlib_actual364_day_counter_hpp

#include <ql/time/daycounter.hpp>

namespace QuantLib {

    //! Actual/364 day count convention
    /*! \ingroup daycounters */
    class Actual364 : public DayCounter {
      private:
        class Impl : public DayCounter::Impl {
          public:
            std::string name() const override { return std::string("Actual/364"); }
            Time
            yearFraction(const Date& d1, const Date& d2, const Date&, const Date&) const override {
                return dayCount(d1,d2)/364.0;
            }
        };
      public:
        Actual364()
        : DayCounter(ext::shared_ptr<DayCounter::Impl>(new Actual364::Impl)) {}
    };

}

#endif


#ifndef id_a827a86b5bc61cc6454f951a20123e7c
#define id_a827a86b5bc61cc6454f951a20123e7c
inline bool test_a827a86b5bc61cc6454f951a20123e7c(int* i) { return i != 0; }
#endif
