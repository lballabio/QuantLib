
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file history.hpp
    \brief history class
*/

#ifndef quantlib_history_h
#define quantlib_history_h

#include <ql/null.hpp>
#include <ql/Utilities/filteringiterator.hpp>
#include <ql/dataformatters.hpp>
#include <vector>

namespace QuantLib {

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
        History(const Date& firstDate, const std::vector<double>& values)
        : firstDate_(firstDate), lastDate_(firstDate + values.size()),
          values_(values) {}
        /*! This constructor initializes the history with the given set of
            values, corresponding to the date range between
            <b><i>firstDate</i></b> and <b><i>lastDate</i></b> included.

            \pre The size of <b><i>values</i></b> must equal the number of
                 days from <b><i>firstDate</i></b> to <b><i>lastDate</i></b>
                 included.
        */
        History(const Date& firstDate, const Date& lastDate,
            const std::vector<double>& values);
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
            const std::vector<double>& values);
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
        double operator[](const Date&) const;
        //@}
        // forward declarations
        class const_iterator;

        //! single datum in history
        class Entry {
            friend class const_iterator;
          public:
            Entry() {
                #if defined(QL_PATCH_DARWIN)
                std::vector<double> v(1,Null<double>());
                #else
                static std::vector<double> v(1,Null<double>());
                #endif
                date_ = Date();
                value_ = v.begin();
            }
            const Date& date() const { return date_; }
            double value() const { return *value_; }
          private:
            Entry(const Date& date,
                const std::vector<double>::const_iterator& value)
            : date_(date), value_(value) {}
            Date date_;
            std::vector<double>::const_iterator value_;
        };

        //! random access iterator on history entries
        class const_iterator : public QL_ITERATOR<
            std::random_access_iterator_tag, Entry,
            int, const Entry*, const Entry&>
        {
            friend class History;
          public:
            /* These typedefs are needed even though inherited from
               QL_ITERATOR (see 14.6.2.3 of the standard).  */
            typedef Entry                           value_type;
            typedef int                             difference_type;
            typedef const Entry*                    pointer;
            typedef const Entry&                    reference;
            //! \name Dereferencing
            //@{
            reference operator*() const  { return entry_; }
            pointer   operator->() const { return &entry_; }
            //@}
            //! \name Random access
            //@{
            value_type operator[](difference_type i) const {
                return Entry(entry_.date_+i,entry_.value_+i);
            }
            //@}
            //! \name Increment and decrement
            //@{
            const_iterator& operator++() {
                entry_.date_++; entry_.value_++;
                return *this;
            }
            const_iterator operator++(int ) {
                const_iterator temp = *this;
                entry_.date_++; entry_.value_++;
                return temp;
            }
            const_iterator& operator--() {
                entry_.date_--; entry_.value_--;
                return *this;
            }
            const_iterator operator--(int ) {
                const_iterator temp = *this;
                entry_.date_--; entry_.value_--;
                return temp;
            }
            const_iterator& operator+=(difference_type i) {
                entry_.date_+=i; entry_.value_+=i;
                return *this;
            }
            const_iterator& operator-=(difference_type i) {
                entry_.date_-=i; entry_.value_-=i;
                return *this;
            }
            const_iterator operator+(difference_type i) {
                return const_iterator(entry_.date_+i,entry_.value_+i);
            }
            const_iterator operator-(difference_type i) {
                return const_iterator(entry_.date_-i,entry_.value_-i);
            }
            //@}
            //! \name Difference
            //@{
            difference_type operator-(const const_iterator& i) {
                return entry_.date_-i.entry_.date_; }
            //@}
            //! \name Comparisons
            //@{
            bool operator==(const const_iterator& i) {
                return entry_.date_ == i.entry_.date_; }
            bool operator!=(const const_iterator& i) {
                return entry_.date_ != i.entry_.date_; }
            bool operator<(const const_iterator& i) {
                return entry_.date_ < i.entry_.date_; }
            bool operator>(const const_iterator& i) {
                return entry_.date_ > i.entry_.date_; }
            bool operator<=(const const_iterator& i) {
                return entry_.date_ <= i.entry_.date_; }
            bool operator>=(const const_iterator& i) {
                return entry_.date_ >= i.entry_.date_; }
            //@}
          private:
            const_iterator(const Date& d,
                const std::vector<double>::const_iterator& v)
            : entry_(d,v) {}
            Entry entry_;
        };

      private:
        class DataValidator;
      public:
        //! bidirectional iterator on non-null history entries
        typedef filtering_iterator<const_iterator,DataValidator>
            const_valid_iterator;

        //! random access iterator on historical data
        typedef std::vector<double>::const_iterator const_data_iterator;

        //! bidirectional iterator on non-null historical data
        typedef filtering_iterator<const_data_iterator,DataValidator>
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
            int i = d-firstDate_;
            return begin()+i;
        }

        // valid entry iterators
        const_valid_iterator vbegin() const {
            return const_valid_iterator(begin(),DataValidator(),
                begin()-1,end());
        }
        const_valid_iterator vend() const {
            return const_valid_iterator(end(),DataValidator(),
                begin()-1,end());
        }
        const_valid_iterator valid_iterator(const Date& d) const {
            return const_valid_iterator(iterator(d),DataValidator(),
                begin()-1,end());
        }

        // data iterators
        const_data_iterator dbegin() const { return values_.begin(); }
        const_data_iterator dend() const { return values_.end(); }
        const_data_iterator data_iterator(const Date& d) const {
            return dbegin()+(d-firstDate_); }

        // valid data iterators
        const_valid_data_iterator vdbegin() const {
            return const_valid_data_iterator(dbegin(),DataValidator(),
                dbegin()-1,dend()); }
        const_valid_data_iterator vdend() const {
            return const_valid_data_iterator(dend(),DataValidator(),
                dbegin()-1,dend()); }
        const_valid_data_iterator valid_data_iterator(const Date& d) const {
            return const_valid_data_iterator(data_iterator(d),
                DataValidator(), dbegin()-1,dend());
        }
        //@}
      private:
        Date firstDate_, lastDate_;
        std::vector<double> values_;
        class DataValidator {
          public:
            bool operator()(double x) {
                return x != Null<double>();
            }
            bool operator()(const Entry& e) {
                return e.value() != Null<double>();
            }
        };
    };


    /*! \example history_iterators.cpp
        This code exemplifies how to use History iterators to perform
        gaussianstatistics analyses on historical data.
    */


    // inline definitions

    inline History::History(const Date& firstDate, const Date& lastDate,
        const std::vector<double>& values)
    : firstDate_(firstDate), lastDate_(lastDate), values_(values) {
        QL_REQUIRE(lastDate >= firstDate, "invalid date range for history");
        QL_REQUIRE(values.size() == Size((lastDate-firstDate)+1),
            "history size incompatible with date range");
    }

    inline History::History(const std::vector<Date>& dates,
        const std::vector<double>& values) {
        QL_REQUIRE(dates.size() == values.size(),
            "different size for date and value vectors");
        QL_REQUIRE(dates.size() >= 1,"null history given");
        firstDate_ = lastDate_ = dates[0];
        double lastValue = values[0];
        values_ = std::vector<double>(1,lastValue);
        for (int i=1; i<static_cast<int>(dates.size()); i++) {
            Date d = dates[i];
            double x = values[i];
            QL_REQUIRE(d>=lastDate_,
                "unsorted date after "+DateFormatter::toString(lastDate_));
            if (d == lastDate_) {
                QL_REQUIRE(x==lastValue,
                    "different values in history for " +
                    DateFormatter::toString(lastDate_));
            } else {
                while (d-lastDate_ > 1) {
                    lastDate_ = lastDate_.plusDays(1);
                    values_.push_back(Null<double>());
                }
                lastDate_ = d;
                values_.push_back(lastValue=x);
            }
        }
    }

    inline double History::operator[](const Date& d) const {
        if (d>=firstDate_ && d<=lastDate_)
            return values_[d-firstDate_];
        else
            return Null<double>();
    }

}


#endif
