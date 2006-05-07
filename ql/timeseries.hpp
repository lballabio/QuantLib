/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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

    //! Container for historical data
    /*! This class acts as a generic repository for a set of historical data.
        Single data can be accessed through their date, while sets of
        consecutive data can be accessed through iterators.
    */
    template <class T>
    class TimeSeries {
      private:
        std::map<Date, T> values_;
      public:
        typedef typename std::vector<T>::const_iterator
                                                   vector_const_iterator_type;
        /*! Default constructor */
        TimeSeries() {}
        /*! This constructor initializes the history with the given set of
            values, corresponding to the date range between
            <b><i>firstDate</i></b> and <b><i>lastDate</i></b> included.

            \pre <b><i>begin</i></b>-<b><i>end</i></b> must equal the
            number of days from <b><i>firstDate</i></b> to
            <b><i>lastDate</i></b> included.
        */
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
        const Date& firstDate() const { return values_.begin()->first; }
        //! returns the last date for which a historical datum exists
        const Date& lastDate() const { return values_.rbegin()->first; }
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
            values_.insert(std::pair<const Date, T>(d, value));
        }
        std::vector<Date> dates() const {
            std::vector<Date> returnval;
            for (const_valid_iterator i = vbegin();
                 i != vend(); i++) {
                returnval.push_back(i->first);
            }
            return returnval;
        }
        std::vector<T> values() const {
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
        for (d = firstDate, i = 0; d <= lastDate; d++, i++) {
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

    //! interval price
    class IntervalPrice {
    public:
        enum Type {Open, Close, High, Low};
        IntervalPrice() :
            open_(QL_NULL_REAL), 
            close_(QL_NULL_REAL), 
            high_(QL_NULL_REAL), 
            low_(QL_NULL_REAL) {};

        IntervalPrice(Real o, Real c, Real h, Real l) :
            open_(o), close_(c), high_(h), low_(l) {};
        void setValue(Real o, Real c, Real h, Real l) {
            open_ = o; close_ = c;
            high_ = h; low_ = l;
        }
        Real open() const {return open_;}
        Real close() const {return close_;}
        Real high() const {return high_;}
        Real low() const {return low_;}
        Real value(IntervalPrice::Type t) const {
            switch(t) {
            case Open:
                return open();
            case Close:
                return close();
            case High:
                return high();
            case Low:
                return low();
            default:
                QL_FAIL("Unknown price type");
            }
        }
    private:
        Real open_, close_, high_, low_;
    };

    //! Create time series of interval prices
    //! This should probably go somewhere other than timeseries
    class TimeSeriesIntervalPriceHelper {  
    public:
        static 
        TimeSeries<IntervalPrice> create(const std::vector<Date>& d,
                                         const std::vector<Real>& open,
                                         const std::vector<Real>& close,
                                         const std::vector<Real>& high,
                                         const std::vector<Real>& low)  {
            Size dsize = d.size();
            QL_REQUIRE((open.size() == dsize &&
                        close.size() == dsize &&
		 high.size() == dsize &&
                        low.size() == dsize),
                       "size mismatch (" << dsize << ", "
                       << open.size() << ", "
                       << close.size() << ", "
                       << high.size() << ", "
                       << low.size() << ")");
            TimeSeries<IntervalPrice> retval;
            std::vector<Date>::const_iterator i;
            std::vector<Real>::const_iterator openi, closei, highi, lowi;
            openi = open.begin();
            closei = close.begin();
            highi = high.begin();
            lowi = low.begin();
            for (i = d.begin(); i != d.end(); i++) {
                retval.insert(*i,
                              IntervalPrice(*openi,
                                            *closei,
                                      *highi,
                                            *lowi));
                openi++; closei++; highi++; lowi++;
            }
            return retval;
        };
        static std::vector<Real> extractValues(const TimeSeries<IntervalPrice> &ts,
                       enum IntervalPrice::Type t)  {
            std::vector<Real> returnval;
            for (TimeSeries<IntervalPrice>::const_valid_iterator i = ts.vbegin();
                 i != ts.vend(); i++) {
                returnval.push_back(i->second.value(t));
            }
            return returnval;
        };

        static TimeSeries<Real> 
        extractComponent(const TimeSeries<IntervalPrice> &ts,
                       enum IntervalPrice::Type t) {
            return TimeSeries<Real>(ts.dates(),
                                    extractValues(ts, t));
        }
    };
}


#endif
