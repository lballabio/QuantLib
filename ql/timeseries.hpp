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

/*! \file timeseries.hpp
    \brief timeseries class
*/

#ifndef quantlib_timeseries_hpp
#define quantlib_timeseries_hpp

#include <ql/date.hpp>
#include <ql/Utilities/null.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <map>
#include <vector>

namespace QuantLib {

    /*! \example timeseries_iterators.cpp
        This code exemplifies how to use TimeSeries iterators to perform
        Gaussian statistic analyses on historical data.
    */

    //! Container for historical data
    /*! This class acts as a generic repository for a set of historical data.
        Single data can be accessed through their date, while sets of
        consecutive data can be accessed through iterators.

        A history can contain null data, which can either be returned or
        skipped according to the chosen iterator type.

        <b>Example: </b>
        \link history_iterators.cpp
        uses of history iterators
        \endlink
    */

    
    template <class T>
    class TimeSeries {
      private:
        std::map<Date, T> values_;
      public:
        /*! Default constructor */
        TimeSeries() {}
        /*! This constructor initializes the history with the given set of
            values, corresponding to the date range between
            <b><i>firstDate</i></b> and <b><i>lastDate</i></b> included.

            \pre <b><i>begin</i></b>-<b><i>end</i></b> must equal the
            number of days from <b><i>firstDate</i></b> to
            <b><i>lastDate</i></b> included.
        */

        typedef typename std::vector<T>::const_iterator
            vector_const_iterator_type;

        template <class Iterator>
        TimeSeries(const Date& firstDate, const Date& lastDate,
                   Iterator begin, Iterator end) {
            Date d = firstDate;
            while (begin != end) {
                values_[d] = *begin;
                ++begin;
                ++d;
            }
            QL_REQUIRE(lastDate >= firstDate,
                       "invalid date range for history");
            QL_ENSURE(values_.size() == Size((lastDate-firstDate)+1),
                      "history size incompatible with date range");
        }
        TimeSeries(const Date& firstDate, const std::vector<T>& values) {
            Date d = firstDate;
            vector_const_iterator_type i = values.begin();
            while (i != values.end()) {
                values_[d] = *i;
                ++i;
                ++d;
            }
            
        }

        /*! This constructor initializes the history with the given set of
            values, corresponding to the date range between
            <b><i>firstDate</i></b> and <b><i>lastDate</i></b> included.

            \pre The size of <b><i>values</i></b> must equal the number of
                 days from <b><i>firstDate</i></b> to <b><i>lastDate</i></b>
                 included.
        */
        TimeSeries(const Date& firstDate, const Date& lastDate,
                const std::vector<T>& values);
        /*! This constructor initializes the history with the given set of
            values, corresponding each to the element with the same index in
            the given set of dates. The whole date range between
            <b><i>dates</i></b>[0] and <b><i>dates</i></b>[N-1] will be
            automatically filled by inserting null values where a date is
            missing from the given set.

            \pre <b><i>dates</i></b> must be sorted.
            \pre There can be no pairs
                 (<b><i>dates</i></b>[i],<b><i>values</i></b>[i]) and
                 (<b><i>dates</i></b>[j],<b><i>values</i></b>[j])
                 such that <tt>dates[i] == dates[j] && values[i] !=
                 values[j]</tt>.
                 Pairs with <tt>dates[i] == dates[j] && values[i] ==
                 values[j]</tt> are allowed; the duplicated entries will be
                 discarded.
            \pre The size of <b><i>values</i></b> must equal the number of
                 days from <b><i>firstDate</i></b> to <b><i>lastDate</i></b>
                 included.
        */
        TimeSeries(const std::vector<Date>& dates,
                const std::vector<T>& values);
        //! \name Inspectors
        //@{
        //! returns the first date for which a historical datum exists
        const Date& firstDate() const { 
            return values_.begin()->first; }
        //! returns the last date for which a historical datum exists
        const Date& lastDate() const { 
            return values_.rbegin()->first; }
        //! returns the number of historical data including null ones
        Size size() const { return values_.size(); }
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
        T& operator[](const Date&d) {
            return values_[d];
        }

        typedef typename std::map<Date,T>::const_iterator 
        const_valid_iterator;

        // valid entry iterators
        const_valid_iterator vbegin() const {
            return values_.begin();
        }
        const_valid_iterator vend() const {
            return values_.end();
        }
        const_valid_iterator valid_iterator(const Date& d) const {
            return values_.find(d);
        }
        void insert(const Date &d, const T &value) {
            values_[d] = value;
        }
        std::vector<Date> dates() {
            std::vector<Date> returnval;
            for (const_valid_iterator i = vbegin();
                 i != vend(); i++) {
                returnval.push_back(i->first);
            }
            return returnval;
        }
        std::vector<T> values() {
            std::vector<T> returnval;
            for (const_valid_iterator i = vbegin();
                 i != vend(); i++) {
                returnval.push_back(i->second);
            }
            return returnval;
        }

        };


    // inline definitions
    template <class T>
    TimeSeries<T>::TimeSeries(const Date& firstDate, 
                              const Date& lastDate,
                              const std::vector<T>& values) {
        QL_REQUIRE(lastDate >= firstDate, "invalid date range for history");
        QL_REQUIRE(values.size() == Size((lastDate-firstDate)+1),
                   "history size incompatible with date range");
        Size i; Date d;
        for(d = firstDate, i = 0; d <= lastDate; d++, i++) {
            values_[d] = values[i];
        }
    }

        template <class T>
    TimeSeries<T>::TimeSeries(const std::vector<Date>& dates,
                            const std::vector<T>& values) {
        QL_REQUIRE(dates.size() == values.size(),
                   "different size for date and value vectors");
        QL_REQUIRE(dates.size() >= 1,"null history given");
        for (Size i=0; i<dates.size(); i++) {
            values_[dates[i]] = values[i];
        }
    }
}


#endif
