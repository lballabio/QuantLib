/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file quote.hpp
    \brief purely virtual base class for market observables
*/

#ifndef quantlib_time_series_hpp
#define quantlib_time_series_hpp

#include <ql/types.hpp>
#include <ql/handle.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    //! Series of quotes
  template <class T>
    class TimeSeries {
        TimeSeries() {};
        TimeSeries(const std::vector<Date> &d,
                   const std::vector<T> & q)
            : dates(d), values(q) {
        }
        std::vector<Date> dates;
        std::vector<T> values;
        void push_back(const Date &d, const T & v) {
            dates.push_back(d);
            values.push_back(v);
        };
    };
}


#endif

