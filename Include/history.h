
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file history.h
    \brief history class

    $Source$
    $Name$
    $Log$
    Revision 1.13  2001/02/13 09:58:23  lballabio
    Some more work on iterators

    Revision 1.12  2001/02/12 19:00:39  lballabio
    Some more work on iterators

    Revision 1.11  2001/02/12 18:34:49  lballabio
    Some work on iterators

    Revision 1.10  2001/02/09 19:16:21  lballabio
    removed QL_PTR_CONST macro

    Revision 1.9  2001/01/23 11:08:51  lballabio
    Renamed iterators in Include\Utilities and related files

    Revision 1.8  2001/01/17 14:37:54  nando
    tabs removed

    Revision 1.7  2001/01/10 11:21:00  lballabio
    Added Examples folder

    Revision 1.6  2001/01/09 17:58:38  enri
    added explicit typedefs to const_iterator

    Revision 1.5  2001/01/09 11:51:51  lballabio
    Using FilteringIterator for valid iterators

    Revision 1.4  2000/12/27 14:18:04  lballabio
    added missing semicolons

    Revision 1.3  2000/12/27 14:05:56  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.2  2000/12/20 15:19:56  lballabio
    Removed History:: scopes not digestible by VC++

    Revision 1.1  2000/12/18 18:37:34  lballabio
    Added to CVS

*/

#ifndef quantlib_history_h
#define quantlib_history_h

#include "qldefines.h"
#include "qlerrors.h"
#include "date.h"
#include "null.h"
#include "filteringiterator.h"
#include "dataformatters.h"
#include <vector>
#include <algorithm>

namespace QuantLib {

    //! Container for historical data
    /*! This class acts as a generic repository for a set of historical data.
        Single data can be accessed through their date, while sets of
        consecutive data can be accessed through iterators.

        A history can contain null data, which can either be returned or skipped
        according to the chosen iterator type.
    */
    class History {
      public:
        /*! Default constructor */
        History() {}
        /*! This constructor initializes the history with the given set of
            values, corresponding to the date range between
            <b><i>firstDate</i></b> and <b><i>lastDate</i></b> included.

            \pre <b><i>begin</i></b>-<b><i>end</i></b> must equal the number of
            days from <b><i>firstDate</i></b> to <b><i>lastDate</i></b>
            included.
        */
        template <class Iterator>
        History(const Date& firstDate, const Date& lastDate,
            Iterator begin, Iterator end)
        : theFirstDate(firstDate), theLastDate(lastDate), theValues(end-begin) {
            QL_REQUIRE(lastDate >= firstDate, "invalid date range for history");
            QL_REQUIRE(values.size() == (lastDate-firstDate)+1,
                "history size incompatible with date range");
            std::copy(begin,end,theValues.begin());
        }
        /*! This constructor initializes the history with the given set of
            values, corresponding to the date range between
            <b><i>firstDate</i></b> and <b><i>lastDate</i></b> included.

            \pre The size of <b><i>values</i></b> must equal the number of days
            from <b><i>firstDate</i></b> to <b><i>lastDate</i></b> included.
        */
        History(const Date& firstDate, const Date& lastDate,
            const std::vector<double>& values);
        /*! This constructor initializes the history with the given set of
            values, corresponding each to the element with the same index in the
            given set of dates. The whole date range between
            <b><i>dates</i></b>[0] and <b><i>dates</i></b>[N-1] will be
            automatically filled by inserting null values where a date is
            missing from the given set.

            \pre <b><i>dates</i></b> must be sorted.
            \pre There can be no pairs
            (<b><i>dates</i></b>[i],<b><i>values</i></b>[i]) and
            (<b><i>dates</i></b>[j],<b><i>values</i></b>[j])
            such that <tt>dates[i] == dates[j] && values[i] != values[j]</tt>.
            Pairs with <tt>dates[i] == dates[j] && values[i] == values[j]</tt>
            are allowed; the duplicated entries will be discarded.
            \pre The size of <b><i>values</i></b> must equal the number of days
            from <b><i>firstDate</i></b> to <b><i>lastDate</i></b> included.
        */
        History(const std::vector<Date>& dates,
            const std::vector<double>& values);
        //! \name Inspectors
        //@{
        //! returns the first date for which a historical datum exists
        const Date& firstDate() const { return theFirstDate; }
        //! returns the last date for which a historical datum exists
        const Date& lastDate() const { return theLastDate; }
        //! returns the number of historical data including null ones
        int size() const { return theValues.size(); }
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
            const Date& date() const { return theDate; }
            double value() const { return *theValue; }
          private:
            Entry(const Date& date,
                const std::vector<double>::const_iterator& value)
            : theDate(date), theValue(value) {}
            Date theDate;
            std::vector<double>::const_iterator theValue;
        };

        //! random access iterator on history entries
        class const_iterator : public QL_ITERATOR<
            std::random_access_iterator_tag, Entry,
            int, const Entry*, const Entry&>
        {
            friend class History;
          public:
            /* it is not really clear (and 14.6.2.3 of the standard doesn't 
               help) whether these typedefs are needed or should be inherited 
               from QL_ITERATOR. Let's play it safe.                       */
            typedef std::random_access_iterator_tag iterator_category;
            typedef Entry                           value_type;
            typedef int                             difference_type;
            typedef const Entry*                    pointer;
            typedef const Entry&                    reference;
            //! \name Dereferencing
            //@{
            reference operator*() const  { return theEntry; }
            pointer   operator->() const { return &theEntry; }
            //@}
            //! \name Random access
            //@{
            value_type operator[](difference_type i) const {
                return Entry(theEntry.theDate+i,theEntry.theValue+i);
            }
            //@}
            //! \name Increment and decrement
            //@{
            const_iterator& operator++() {
                theEntry.theDate++; theEntry.theValue++;
                return *this;
            }
            const_iterator operator++(int) {
                const_iterator temp = *this;
                theEntry.theDate++; theEntry.theValue++;
                return temp;
            }
            const_iterator& operator--() {
                theEntry.theDate--; theEntry.theValue--;
                return *this;
            }
            const_iterator operator--(int) {
                const_iterator temp = *this;
                theEntry.theDate--; theEntry.theValue--;
                return temp;
            }
            const_iterator& operator+=(difference_type i) {
                theEntry.theDate+=i; theEntry.theValue+=i;
                return *this;
            }
            const_iterator& operator-=(difference_type i) {
                theEntry.theDate-=i; theEntry.theValue-=i;
                return *this;
            }
            const_iterator operator+(difference_type i) {
                return const_iterator(theEntry.theDate+i,theEntry.theValue+i);
            }
            const_iterator operator-(difference_type i) {
                return const_iterator(theEntry.theDate-i,theEntry.theValue-i);
            }
            //@}
            //! \name Difference
            //@{
            difference_type operator-(const const_iterator& i) {
                return theEntry.theDate-i.theEntry.theDate; }
            //@}
            //! \name Comparisons
            //@{
            bool operator==(const const_iterator& i) {
                return theEntry.theValue == i.theEntry.theValue; }
            bool operator!=(const const_iterator& i) {
                return theEntry.theValue != i.theEntry.theValue; }
            bool operator<(const const_iterator& i) {
                return theEntry.theValue < i.theEntry.theValue; }
            bool operator>(const const_iterator& i) {
                return theEntry.theValue > i.theEntry.theValue; }
            bool operator<=(const const_iterator& i) {
                return theEntry.theValue <= i.theEntry.theValue; }
            bool operator>=(const const_iterator& i) {
                return theEntry.theValue >= i.theEntry.theValue; }
            //@}
          private:
            const_iterator(const Date& d,
                const std::vector<double>::const_iterator& v)
            : theEntry(d,v) {}
            Entry theEntry;
        };

      private:
        class DataValidator;
      public:
        //! bidirectional iterator on non-null history entries
        typedef Utilities::filtering_iterator<const_iterator,DataValidator>
            const_valid_iterator;

        //! random access iterator on historical data
        typedef std::vector<double>::const_iterator const_data_iterator;

        //! bidirectional iterator on non-null historical data
        typedef Utilities::filtering_iterator<const_data_iterator,DataValidator>
            const_valid_data_iterator;

        /*! \name Iterator access
            Four different types of iterators are provided, namely,
            const_iterator, const_valid_iterator, const_data_iterator, and
            const_valid_data_iterator.

            const_iterator and const_valid_iterator point to an Entry structure,
            the difference being that the latter only iterates over valid
            entries - i.e., entries whose data are not null.
            The same difference exists between const_data_iterator and
            const_valid_data_iterator which point directly to historical values
            without reference to the date they are associated to.
        */
        //@{
        // entry iterators
        const_iterator begin() const {
            return const_iterator(theFirstDate,theValues.begin()); }
        const_iterator end() const {
            return const_iterator(theLastDate+1,theValues.end()); }
        const_iterator iterator(const Date& d) const {
            int i = d-theFirstDate;
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
        const_data_iterator dbegin() const { return theValues.begin(); }
        const_data_iterator dend() const { return theValues.end(); }
        const_data_iterator data_iterator(const Date& d) const {
            return dbegin()+(d-theFirstDate); }

        // valid data iterators
        const_valid_data_iterator vdbegin() const {
            return const_valid_data_iterator(dbegin(),DataValidator(),
                dbegin()-1,dend()); }
        const_valid_data_iterator vdend() const {
            return const_valid_data_iterator(dend(),DataValidator(),
                dbegin()-1,dend()); }
        const_valid_data_iterator valid_data_iterator(const Date& d) const {
            return const_valid_data_iterator(data_iterator(d),DataValidator(),
            dbegin()-1,dend());
        }
        //@}
      private:
        Date theFirstDate, theLastDate;
        std::vector<double> theValues;
        class DataValidator {
          public:
            bool operator()(double x)       { return !IsNull(x); }
            bool operator()(const Entry& e) { return !IsNull(e.value()); }
        };
    };


    /*! \example history_iterators.cpp
        This code exemplifies how to use History iterators to perform statistic
        analyses on historical data.
    */


    // inline definitions

    inline History::History(const Date& firstDate, const Date& lastDate,
        const std::vector<double>& values)
    : theFirstDate(firstDate), theLastDate(lastDate), theValues(values) {
        QL_REQUIRE(lastDate >= firstDate, "invalid date range for history");
        QL_REQUIRE(values.size() == (lastDate-firstDate)+1,
            "history size incompatible with date range");
    }

    inline History::History(const std::vector<Date>& dates,
        const std::vector<double>& values) {
        QL_REQUIRE(dates.size() == values.size(),
            "different size for date and value vectors");
        QL_REQUIRE(dates.size() >= 1,"null history given");
        theFirstDate = theLastDate = dates[0];
        double lastValue = values[0];
        theValues = std::vector<double>(1,lastValue);
        for (int i=1; i<dates.size(); i++) {
            Date d = dates[i];
            double x = values[i];
            QL_REQUIRE(d>=theLastDate,
                "unsorted date after "+DateFormatter::toString(theLastDate));
            if (d == theLastDate) {
                QL_REQUIRE(x==lastValue,
                    "different values in history for " +
                    DateFormatter::toString(theLastDate));
            } else {
                while (d-theLastDate > 1) {
                    theLastDate = theLastDate.plusDays(1);
                    theValues.insert(theValues.end(),Null<double>());
                }
                theLastDate = d;
                theValues.insert(theValues.end(),lastValue=x);
            }
        }
    }

    inline double History::operator[](const Date& d) const {
        if (d>=theFirstDate && d<=theLastDate)
            return theValues[d-theFirstDate];
        else
            return Null<double>();
    }

}


#endif
