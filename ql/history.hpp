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

/*! \file history.hpp
    \brief history class
*/

#ifndef quantlib_history_hpp
#define quantlib_history_hpp

#include <ql/date.hpp>
#include <ql/Utilities/null.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <vector>

namespace QuantLib {

    /*! \example history_iterators.cpp
        This code exemplifies how to use History iterators to perform
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
    class History {
      public:
        /*! Default constructor */
        History() {}
        /*! This constructor initializes the history with the given set of
            values, corresponding to the date range between
            <b><i>firstDate</i></b> and <b><i>lastDate</i></b> included.

            \pre <b><i>begin</i></b>-<b><i>end</i></b> must equal the
            number of days from <b><i>firstDate</i></b> to
            <b><i>lastDate</i></b> included.
        */
        template <class Iterator>
        History(const Date& firstDate, const Date& lastDate,
                Iterator begin, Iterator end)
        : firstDate_(firstDate), lastDate_(lastDate)
        #if defined(QL_FULL_ITERATOR_SUPPORT)
        , values_(begin,end) {
        #else
        {
            while (begin != end) {
                values_.push_back(*begin);
                ++begin;
            }
        #endif
            QL_REQUIRE(lastDate >= firstDate,
                       "invalid date range for history");
            QL_ENSURE(values_.size() == Size((lastDate-firstDate)+1),
                      "history size incompatible with date range");
        }
        History(const Date& firstDate, const std::vector<Real>& values)
        : firstDate_(firstDate),
          lastDate_(firstDate + BigInteger(values.size())),
          values_(values) {}
        /*! This constructor initializes the history with the given set of
            values, corresponding to the date range between
            <b><i>firstDate</i></b> and <b><i>lastDate</i></b> included.

            \pre The size of <b><i>values</i></b> must equal the number of
                 days from <b><i>firstDate</i></b> to <b><i>lastDate</i></b>
                 included.
        */
        History(const Date& firstDate, const Date& lastDate,
                const std::vector<Real>& values);
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
        History(const std::vector<Date>& dates,
                const std::vector<Real>& values);
        //! \name Inspectors
        //@{
        //! returns the first date for which a historical datum exists
        const Date& firstDate() const { return firstDate_; }
        //! returns the last date for which a historical datum exists
        const Date& lastDate() const { return lastDate_; }
        //! returns the number of historical data including null ones
        Size size() const { return values_.size(); }
        //@}
        //! \name Historical data access
        //@{
        //! returns the (possibly null) datum corresponding to the given date
        Real operator[](const Date&) const;
        //@}
        // forward declarations
        class const_iterator;

        //! single datum in history
        class Entry {
            friend class const_iterator;
          public:
            Entry() {
                #if defined(QL_PATCH_DARWIN)
                std::vector<Real> v(1,Null<Real>());
                #else
                static std::vector<Real> v(1,Null<Real>());
                #endif
                date_ = Date();
                value_ = v.begin();
            }
            const Date& date() const { return date_; }
            Real value() const { return *value_; }
          private:
            Entry(const Date& date,
                  const std::vector<Real>::const_iterator& value)
            : date_(date), value_(value) {}
            Date date_;
            std::vector<Real>::const_iterator value_;
        };

        //! random access iterator on history entries
        class const_iterator
            : public boost::iterator_facade<const_iterator,
                                            Entry,
                                            std::random_access_iterator_tag,
                                            const Entry&,
                                            BigInteger> {
            friend class History;
          private:
            // only histories can build them
            const_iterator(const Date& d,
                           const std::vector<Real>::const_iterator& v)
            : entry_(d,v) {}
            Entry entry_;
          public:
            // iterator_facade interface
            const Entry& dereference() const {
                return entry_;
            }
            bool equal(const const_iterator& i) const {
                return entry_.date_ == i.entry_.date_;
            }
            void increment() {
                entry_.date_++;
                entry_.value_++;
            }
            void decrement() {
                entry_.date_--;
                entry_.value_--;
            }
            void advance(BigInteger n) {
                entry_.date_ += n;
                entry_.value_ += n;
            }
            BigInteger distance_to(const const_iterator& i) const {
                return i.entry_.date_ - entry_.date_;
            }
        };

      private:
        class DataValidator;
      public:
        //! bidirectional iterator on non-null history entries
        typedef boost::filter_iterator<DataValidator,const_iterator>
            const_valid_iterator;

        //! random access iterator on historical data
        typedef std::vector<Real>::const_iterator const_data_iterator;

        //! bidirectional iterator on non-null historical data
        typedef boost::filter_iterator<DataValidator,const_data_iterator>
            const_valid_data_iterator;

        /*! \name Iterator access
            Four different types of iterators are provided, namely,
            const_iterator, const_valid_iterator, const_data_iterator, and
            const_valid_data_iterator.

            const_iterator and const_valid_iterator point to an Entry
            structure, the difference being that the latter only iterates
            over valid entries - i.e., entries whose data are not null.
            The same difference exists between const_data_iterator and
            const_valid_data_iterator which point directly to historical
            values without reference to the date they are associated to.
        */
        //@{
        // entry iterators
        const_iterator begin() const {
            return const_iterator(firstDate_,values_.begin()); }
        const_iterator end() const {
            return const_iterator(lastDate_+1,values_.end()); }
        const_iterator iterator(const Date& d) const {
            BigInteger i = d-firstDate_;
            return begin()+i;
        }

        // valid entry iterators
        const_valid_iterator vbegin() const {
            return const_valid_iterator(DataValidator(),begin(),end());
        }
        const_valid_iterator vend() const {
            return const_valid_iterator(DataValidator(),end(),end());
        }
        const_valid_iterator valid_iterator(const Date& d) const {
            return const_valid_iterator(DataValidator(),iterator(d),end());
        }

        // data iterators
        const_data_iterator dbegin() const { return values_.begin(); }
        const_data_iterator dend() const { return values_.end(); }
        const_data_iterator data_iterator(const Date& d) const {
            return dbegin()+(d-firstDate_); }

        // valid data iterators
        const_valid_data_iterator vdbegin() const {
            return const_valid_data_iterator(DataValidator(),dbegin(),dend());
        }
        const_valid_data_iterator vdend() const {
            return const_valid_data_iterator(DataValidator(),dend(),dend());
        }
        const_valid_data_iterator valid_data_iterator(const Date& d) const {
            return const_valid_data_iterator(DataValidator(),
                                             data_iterator(d),dend());
        }
        //@}
      private:
        Date firstDate_, lastDate_;
        std::vector<Real> values_;
        class DataValidator {
          public:
            bool operator()(Real x) {
                return x != Null<Real>();
            }
            bool operator()(const Entry& e) {
                return e.value() != Null<Real>();
            }
        };
    };


    // inline definitions

    inline History::History(const Date& firstDate, const Date& lastDate,
                            const std::vector<Real>& values)
    : firstDate_(firstDate), lastDate_(lastDate), values_(values) {
        QL_REQUIRE(lastDate >= firstDate, "invalid date range for history");
        QL_REQUIRE(values.size() == Size((lastDate-firstDate)+1),
                   "history size incompatible with date range");
    }

    inline History::History(const std::vector<Date>& dates,
                            const std::vector<Real>& values) {
        QL_REQUIRE(dates.size() == values.size(),
                   "different size for date and value vectors");
        QL_REQUIRE(dates.size() >= 1,"null history given");
        firstDate_ = lastDate_ = dates[0];
        Real lastValue = values[0];
        values_ = std::vector<Real>(1,lastValue);
        for (Size i=1; i<dates.size(); i++) {
            Date d = dates[i];
            Real x = values[i];
            QL_REQUIRE(d>=lastDate_,
                       "unsorted date after " << lastDate_);
            if (d == lastDate_) {
                QL_REQUIRE(x==lastValue,
                           "different values in history for " << lastDate_);
            } else {
                while (d > lastDate_ + 1) {
                    ++lastDate_;
                    values_.push_back(Null<Real>());
                }
                lastDate_ = d;
                values_.push_back(lastValue=x);
            }
        }
    }

    inline Real History::operator[](const Date& d) const {
        if (d>=firstDate_ && d<=lastDate_)
            return values_[d-firstDate_];
        else
            return Null<Real>();
    }

}


#endif
