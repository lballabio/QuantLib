/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Decillion Pty(Ltd)
 Copyright (C) 2003 StatPro Italia srl

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

/*! \file timebasket.hpp
    \brief distribution over a number of date ranges
*/

#ifndef quantlib_time_basket_hpp
#define quantlib_time_basket_hpp

#include <ql/time/date.hpp>
#include <ql/utilities/null.hpp>
#include <vector>
#include <map>

namespace QuantLib {

    //! Distribution over a number of dates
    class TimeBasket : private std::map<Date,Real> {
        // this is needed for Visual C++ 6
        typedef std::map<Date,Real> super;
      public:
        TimeBasket() = default;
        TimeBasket(const std::vector<Date>& dates,
                   const std::vector<Real>& values);
        //! \name Map interface
        //@{
        //! returns the number of entries
        using super::size;
        //! element access
        using super::operator[];
        // iterators
        typedef super::iterator iterator;
        typedef super::const_iterator const_iterator;
        typedef super::reverse_iterator reverse_iterator;
        typedef super::const_reverse_iterator const_reverse_iterator;
        using super::begin;
        using super::end;
        using super::rbegin;
        using super::rend;
        //! membership
        bool hasDate(const Date&) const;
        //@}
        //! \name Algebra
        //@{
        TimeBasket& operator+=(const TimeBasket& other);
        TimeBasket& operator-=(const TimeBasket& other);
        //@}
        //! \name Other methods
        //@{
        //! redistribute the entries over the given dates
        TimeBasket rebin(const std::vector<Date>& buckets) const;
        //@}
    };


    // inline definitions

    inline bool TimeBasket::hasDate(const Date& d) const {
        auto i = find(d);
        return i != end();
    }

    inline TimeBasket& TimeBasket::operator+=(const TimeBasket& other) {
        super& self = *this;
        for (auto j : other)
            self[j.first] += j.second;
        return *this;
    }

    inline TimeBasket& TimeBasket::operator-=(const TimeBasket& other) {
        super& self = *this;
        for (auto j : other)
            self[j.first] -= j.second;
        return *this;
    }

}


#endif
