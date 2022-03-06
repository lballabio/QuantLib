/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang
 Copyright (C) 2010 Liquidnet Holdings, Inc.

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
#include <ql/functional.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/reverse_iterator.hpp>
#include <boost/utility.hpp>
#include <algorithm>
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
        TimeSeries() = default;
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
        typedef typename const_iterator::iterator_category iterator_category;

        // Reverse iterators
        // The following class makes compilation fail for the code
        // that calls rbegin or rend with a container that does not
        // support reverse iterators.  All the rest TimeSeries class
        // features should compile and work for this type of
        // containers.
        template <class container, class iterator_category>
        struct reverse {
            typedef boost::reverse_iterator<typename container::const_iterator>
                                                       const_reverse_iterator;
            reverse(const container& c) : c_(c) {}
            const_reverse_iterator rbegin() const {
                return const_reverse_iterator(c_.end());
            }
            const_reverse_iterator rend() const {
                return const_reverse_iterator(c_.begin());
            }
            const container& c_;
        };

        // This class defines reverse iterator features via
        // container's native calls.
        template <class container>
        struct reverse<container, std::bidirectional_iterator_tag> {
            typedef typename container::const_reverse_iterator
                                                       const_reverse_iterator;
            reverse(const container& c) : c_(c) {}
            const_reverse_iterator rbegin() const { return c_.rbegin(); }
            const_reverse_iterator rend() const { return c_.rend(); }
            const container& c_;
        };

        // The following typedef enables reverse iterators for
        // bidirectional_iterator_tag category.
        typedef typename boost::mpl::if_ <
            boost::mpl::or_ <
                boost::is_same<iterator_category,
                               std::bidirectional_iterator_tag>,
                boost::is_base_of<std::bidirectional_iterator_tag,
                                  iterator_category> >,
            std::bidirectional_iterator_tag, 
            std::input_iterator_tag>::type enable_reverse;

        typedef typename
        reverse<Container, enable_reverse>::const_reverse_iterator
                                                       const_reverse_iterator;

        const_iterator cbegin() const;
        const_iterator cend() const;
        const_iterator begin() const { return cbegin(); }
        const_iterator end() const { return cend(); }
        const_reverse_iterator crbegin() const {
            return reverse<Container, enable_reverse>(values_).rbegin();
        }
        const_reverse_iterator crend() const {
            return reverse<Container, enable_reverse>(values_).rend();
        }
        const_reverse_iterator rbegin() const { return crbegin(); }
        const_reverse_iterator rend() const { return crend(); }
        //@}

      private:
        typedef typename Container::value_type container_value_type;
        typedef ext::function<Date(const container_value_type&)>
                                                              projection_time;
        typedef ext::function<T(const container_value_type&)>
                                                             projection_value;

      public:
        //! \name Projection iterators
        //@{

        typedef boost::transform_iterator<projection_time, const_iterator>
                                                          const_time_iterator;
        typedef boost::transform_iterator<projection_value, const_iterator>
                                                         const_value_iterator;
        typedef boost::transform_iterator<projection_time,
                                          const_reverse_iterator>
                                                  const_reverse_time_iterator;
        typedef boost::transform_iterator<projection_value,
                                          const_reverse_iterator>
                                                 const_reverse_value_iterator;

        const_value_iterator cbegin_values() const {
            return const_value_iterator(cbegin(), get_value);
        }
        const_value_iterator cend_values() const {
            return const_value_iterator(cend(), get_value);
        }
        const_reverse_value_iterator crbegin_values() const {
            return const_reverse_value_iterator(crbegin(), get_value);
        }
        const_reverse_value_iterator crend_values() const {
            return const_reverse_value_iterator(crend(), get_value);
        }

        const_time_iterator cbegin_time() const {
            return const_time_iterator(cbegin(), get_time);
        }
        const_time_iterator cend_time() const {
            return const_time_iterator(cend(), get_time);
        }
        const_reverse_time_iterator crbegin_time() const {
            return const_reverse_time_iterator(crbegin(), get_time);
        }
        const_reverse_time_iterator crend_time() const {
            return const_reverse_time_iterator(crend(), get_time);
        }

        //! \name Utilities
        //@{
        const_iterator find(const Date&);
        //! returns the dates for which historical data exist
        std::vector<Date> dates() const;
        //! returns the historical data
        std::vector<T> values() const;
        //@}

      private:
        static const Date& get_time (const container_value_type& v) {
            return v.first;
        }
        static const T& get_value (const container_value_type& v) {
            return v.second;
        }
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
        return rbegin()->first;
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
    TimeSeries<T,C>::cbegin() const {
        return values_.begin();
    }

    template <class T, class C>
    inline typename TimeSeries<T,C>::const_iterator
    TimeSeries<T,C>::cend() const {
        return values_.end();
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
        std::transform(cbegin(), cend(), std::back_inserter(v),
                       TimeSeries<T,C>::get_time);
        return v;
    }

    template <class T, class C>
    std::vector<T> TimeSeries<T,C>::values() const {
        std::vector<T> v;
        v.reserve(size());
        std::transform(cbegin(), cend(), std::back_inserter(v),
                       TimeSeries<T,C>::get_value);
        return v;
    }

}

#endif


#ifndef id_06fd40b0bd930e45de5a7756f567e00a
#define id_06fd40b0bd930e45de5a7756f567e00a
inline bool test_06fd40b0bd930e45de5a7756f567e00a(int* i) { return i != 0; }
#endif
