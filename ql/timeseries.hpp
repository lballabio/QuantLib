/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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

/*! \file timeseries.hpp
    \brief Container for historical data
*/

#ifndef quantlib_timeseries_hpp
#define quantlib_timeseries_hpp

#include <ql/time/date.hpp>
#include <ql/utilities/null.hpp>
#include <ql/errors.hpp>
#include <map>
#include <vector>

namespace QuantLib {

    //! Container for historical data
    /*! This class acts as a generic repository for a set of
        historical data.  Any single datum can be accessed through its
        date, while sets of consecutive data can be accessed through
        iterators.

        \pre The <c>Container</c> type must satisfy the requirements
             set by the C++ standard for associative containers.
    */
    template <class T, class Container = std::map<Date, T> >
    class TimeSeries {
      public:
        typedef Date key_type;
        typedef T value_type;
      private:
        mutable Container values_;
      public:
        /*! Default constructor */
        TimeSeries() {}
        /*! This constructor initializes the history with a set of
            values passed as two sequences, the first containing dates
            and the second containing corresponding values.
        */
        template <class DateIterator, class ValueIterator>
        TimeSeries(DateIterator dBegin, DateIterator dEnd,
                   ValueIterator vBegin) {
            while (dBegin != dEnd)
                values_[*(dBegin++)] = *(vBegin++);
        }
        /*! This constructor initializes the history with a set of
            values. Such values are assigned to a corresponding number
            of consecutive dates starting from <b><i>firstDate</i></b>
            included.
        */
        template <class ValueIterator>
        TimeSeries(const Date& firstDate,
                   ValueIterator begin, ValueIterator end) {
            Date d = firstDate;
            while (begin != end)
                values_[d++] = *(begin++);
        }
        //! \name Inspectors
        //@{
        //! returns the first date for which a historical datum exists
        Date firstDate() const;
        //! returns the last date for which a historical datum exists
        Date lastDate() const;
        //! returns the number of historical data including null ones
        Size size() const;
        //! returns whether the series contains any data
        bool empty() const;
        //@}
        //! \name Historical data access
        //@{
        //! returns the (possibly null) datum corresponding to the given date
        T operator[](const Date& d) const {
            if (values_.find(d) != values_.end())
                return values_[d];
            else
                return Null<T>();
        }
        T& operator[](const Date& d) {
            if (values_.find(d) == values_.end())
                values_[d] = Null<T>();
            return values_[d];
        }
        //@}

        //! \name Iterators
        //@{
        typedef typename Container::const_iterator const_iterator;
        typedef typename Container::const_reverse_iterator
                                           const_reverse_iterator;
        const_iterator begin() const;
        const_iterator end() const;
        const_reverse_iterator rbegin() const;
        const_reverse_iterator rend() const;
        //@}

        //! \name Utilities
        //@{
        const_iterator find(const Date&);
        //! returns the dates for which historical data exist
        std::vector<Date> dates() const;
        //! returns the historical data
        std::vector<T> values() const;
        //@}
    };


    // inline definitions

    template <class T, class C>
    inline Date TimeSeries<T,C>::firstDate() const {
        QL_REQUIRE(!values_.empty(), "empty timeseries");
        return values_.begin()->first;
    }

    template <class T, class C>
    inline Date TimeSeries<T,C>::lastDate() const {
        QL_REQUIRE(!values_.empty(), "empty timeseries");
        return values_.rbegin()->first;
    }

    template <class T, class C>
    inline Size TimeSeries<T,C>::size() const {
        return values_.size();
    }

    template <class T, class C>
    inline bool TimeSeries<T,C>::empty() const {
        return values_.empty();
    }

    template <class T, class C>
    inline typename TimeSeries<T,C>::const_iterator
    TimeSeries<T,C>::begin() const {
        return values_.begin();
    }

    template <class T, class C>
    inline typename TimeSeries<T,C>::const_iterator
    TimeSeries<T,C>::end() const {
        return values_.end();
    }

    template <class T, class C>
    inline typename TimeSeries<T,C>::const_reverse_iterator
    TimeSeries<T,C>::rbegin() const {
        return values_.rbegin();
    }

    template <class T, class C>
    inline typename TimeSeries<T,C>::const_reverse_iterator
    TimeSeries<T,C>::rend() const {
        return values_.rend();
    }

    template <class T, class C>
    inline typename TimeSeries<T,C>::const_iterator
    TimeSeries<T,C>::find(const Date& d) {
        const_iterator i = values_.find(d);
        if (i == values_.end()) {
            values_[d] = Null<T>();
            i = values_.find(d);
        }
        return i;
    }

    template <class T, class C>
    std::vector<Date> TimeSeries<T,C>::dates() const {
        std::vector<Date> v;
        v.reserve(size());
        for (const_iterator i = begin(); i != end(); ++i)
            v.push_back(i->first);
        return v;
    }

    template <class T, class C>
    std::vector<T> TimeSeries<T,C>::values() const {
        std::vector<T> v;
        v.reserve(size());
        for (const_iterator i = begin(); i != end(); ++i)
            v.push_back(i->second);
        return v;
    }

}

#endif
