
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file history.h
	\brief history class
	
	$Source$
	$Name$
	$Log$
	Revision 1.1  2000/12/18 18:37:34  lballabio
	Added to CVS

*/

#ifndef quantlib_history_h
#define quantlib_history_h

#include "qldefines.h"
#include "qlerrors.h"
#include "date.h"
#include "null.h"
#include "dataformatters.h"
#include <vector>
#include <algorithm>

namespace QuantLib {

	//! Container for historical data
	/*! This class acts as a generic repository for a set of historical data. Single data can be accessed
		through their date, while sets of consecutive data can be accessed through iterators.
		
		A history can contain null data, which can either be returned or skipped according to the chosen
		iterator type.
	*/
	class History {
	  public:
		/*! Default constructor */
		History() {}
		/*! This constructor initializes the history with the given set of values, corresponding to the
			date range between <b><i>firstDate</i></b> and <b><i>lastDate</i></b> included.
			\pre <b><i>begin</i></b>-<b><i>end</i></b> must equal the number of days from <b><i>firstDate</i></b>
			to <b><i>lastDate</i></b> included.
		*/
		template <class Iterator>
		History(const Date& firstDate, const Date& lastDate, Iterator begin, Iterator end)
		: theFirstDate(firstDate), theLastDate(lastDate), theValues(end-begin) {
			Require(lastDate >= firstDate, "invalid date range for history");
			Require(values.size() == (lastDate-firstDate)+1, "history size incompatible with date range");
			std::copy(begin,end,theValues.begin());
		}
		/*! This constructor initializes the history with the given set of values, corresponding to the
			date range between <b><i>firstDate</i></b> and <b><i>lastDate</i></b> included.
			\pre The size of <b><i>values</i></b> must equal the number of days from <b><i>firstDate</i></b>
			to <b><i>lastDate</i></b> included.
		*/
		History(const Date& firstDate, const Date& lastDate, const std::vector<double>& values);
		/*! This constructor initializes the history with the given set of values, corresponding each
			to the element with the same index in the given set of dates. The whole date range between
			<b><i>dates</i></b>[0] and <b><i>dates</i></b>[N-1] will be automatically filled by inserting 
			null values where a date is missing from the given set.
			\pre <b><i>dates</i></b> must be sorted.
			\pre There can be no pairs (<b><i>dates</i></b>[i],<b><i>values</i></b>[i]) and 
			(<b><i>dates</i></b>[j],<b><i>values</i></b>[j])
			such that <tt>dates[i] == dates[j] && values[i] != values[j]</tt>. Pairs with 
			<tt>dates[i] == dates[j] && values[i] == values[j]</tt> are allowed; the duplicated entries 
			will be discarded.
			\pre The size of <b><i>values</i></b> must equal the number of days from <b><i>firstDate</i></b> 
			to <b><i>lastDate</i></b> included.
		*/
		History(const std::vector<Date>& dates, const std::vector<double>& values);
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
		class const_valid_iterator;
		//! single datum in history
		class Entry {
			friend class History::const_iterator;
			friend class History::const_valid_iterator;
		  public:
			const Date& date() const { return theDate; }
			double value() const { return *theValue; }
		  private:
			Entry(const Date& date, const std::vector<double>::const_iterator& value)
			: theDate(date), theValue(value) {}
			Date theDate;
			std::vector<double>::const_iterator theValue;
		};
		//! const iterator on history entries
		class const_iterator : public std::iterator<std::random_access_iterator_tag,History::Entry> {
			friend class History;
		  public:
			//! \name Dereferencing
			//@{
			const History::Entry& operator*() const { return theEntry; }
			const History::Entry QL_PTR_CONST operator->() const { return &theEntry; }
			//@}
			//! \name Random access
			//@{
			History::Entry operator[](int i) const {
				return History::Entry(theEntry.theDate+i,theEntry.theValue+i);
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
			const_iterator& operator+=(int i) {
				theEntry.theDate+=i; theEntry.theValue+=i;
				return *this;
			}
			const_iterator& operator-=(int i) {
				theEntry.theDate-=i; theEntry.theValue-=i;
				return *this;
			}
			const_iterator operator+(int i) {
				return const_iterator(theEntry.theDate+i,theEntry.theValue+i);
			}
			const_iterator operator-(int i) {
				return const_iterator(theEntry.theDate-i,theEntry.theValue-i);
			}
			//@}
			//! \name Difference
			//@{
			int operator-(const const_iterator& i) { return theEntry.theValue-i.theEntry.theValue; }
			//@}
			//! \name Comparisons
			//@{
			bool operator==(const const_iterator& i) { return theEntry.theValue == i.theEntry.theValue; }
			bool operator!=(const const_iterator& i) { return theEntry.theValue != i.theEntry.theValue; }
			bool operator<(const const_iterator& i) { return theEntry.theValue < i.theEntry.theValue; }
			bool operator>(const const_iterator& i) { return theEntry.theValue > i.theEntry.theValue; }
			bool operator<=(const const_iterator& i) { return theEntry.theValue <= i.theEntry.theValue; }
			bool operator>=(const const_iterator& i) { return theEntry.theValue >= i.theEntry.theValue; }
			//@}
		  private:
			const_iterator(const Date& d, const std::vector<double>::const_iterator& v)
			: theEntry(d,v) {}
			History::Entry theEntry;
		};
		
		//! const iterator on non-null history entries
		class const_valid_iterator : public std::iterator<std::bidirectional_iterator_tag,History::Entry> {
			friend class History;
		  public:
			//! \name Dereferencing
			//@{
			const History::Entry& operator*() const { return theEntry; }
			const History::Entry QL_PTR_CONST operator->() const { return &theEntry; }
			//@}
			//! \name Increment and decrement
			//@{
			const_valid_iterator& operator++() {
				do {
					theEntry.theDate++; theEntry.theValue++;
				} while (theEntry.theValue < end && IsNull(*(theEntry.theValue)));
				return *this;
			}
			const_valid_iterator operator++(int) {
				const_valid_iterator temp = *this;
				do {
					theEntry.theDate++; theEntry.theValue++;
				} while (theEntry.theValue < end && IsNull(*(theEntry.theValue)));
				return temp;
			}
			const_valid_iterator& operator--() {
				do {
					theEntry.theDate--; theEntry.theValue--;
				} while (theEntry.theValue > beforeBegin && IsNull(*(theEntry.theValue)));
				return *this;
			}
			const_valid_iterator operator--(int) {
				const_valid_iterator temp = *this;
				do {
					theEntry.theDate--; theEntry.theValue--;
				} while (theEntry.theValue > beforeBegin && IsNull(*(theEntry.theValue)));
				return temp;
			}
			//@}
			//! \name Comparisons
			//@{
			bool operator==(const const_valid_iterator& i) { return theEntry.theValue == i.theEntry.theValue; }
			bool operator!=(const const_valid_iterator& i) { return theEntry.theValue != i.theEntry.theValue; }
			//@}
		  private:
			const_valid_iterator(const Date& d, const std::vector<double>::const_iterator& v, 
			  const std::vector<double>::const_iterator& begin, const std::vector<double>::const_iterator& end)
			: beforeBegin(begin-1), end(end), theEntry(d,v) {
				while (theEntry.theValue < end && IsNull(*(theEntry.theValue))) {
					theEntry.theDate++; theEntry.theValue++;
				}
			}
			std::vector<double>::const_iterator beforeBegin, end;
			History::Entry theEntry;
		};

		typedef std::vector<double>::const_iterator const_data_iterator;
		
		//! const iterator on non-null history entries
		class const_valid_data_iterator : public std::iterator<std::bidirectional_iterator_tag,double> {
			friend class History;
		  public:
			//! \name Dereferencing
			//@{
			double operator*() const { return *iter; }
			//@}
			//! \name Increment and decrement
			//@{
			const_valid_data_iterator& operator++() {
				do {
					iter++;
				} while (iter < end && IsNull(*iter));
				return *this;
			}
			const_valid_data_iterator operator++(int) {
				const_valid_data_iterator temp = *this;
				do {
					iter++;
				} while (iter < end && IsNull(*iter));
				return temp;
			}
			const_valid_data_iterator& operator--() {
				do {
					iter--;
				} while (iter > beforeBegin && IsNull(*iter));
				return *this;
			}
			const_valid_data_iterator operator--(int) {
				const_valid_data_iterator temp = *this;
				do {
					iter--;
				} while (iter > beforeBegin && IsNull(*iter));
				return temp;
			}
			//@}
			//! \name Comparisons
			//@{
			bool operator==(const const_valid_data_iterator& i) { return iter == i.iter; }
			bool operator!=(const const_valid_data_iterator& i) { return iter != i.iter; }
			//@}
		  private:
			const_valid_data_iterator(const std::vector<double>::const_iterator& it, 
			  const std::vector<double>::const_iterator& begin, const std::vector<double>::const_iterator& end)
			: iter(it), beforeBegin(begin-1), end(end) {
				while (iter < end && IsNull(*iter))
					iter++;
			}
			std::vector<double>::const_iterator iter, beforeBegin, end;
		};
		
		//! \name Iterator access
		//@{
		// entry iterators
		const_iterator begin() const { return const_iterator(theFirstDate,theValues.begin()); }
		const_iterator end() const { return const_iterator(theLastDate+1,theValues.end()); }
		const_iterator iterator(const Date& d) const {
			int i = d-theFirstDate;
			return begin()+i;
		}
		// valid entry iterators
		const_valid_iterator vbegin() const { 
			return const_valid_iterator(theFirstDate,theValues.begin(),theValues.begin(),theValues.end()); }
		const_valid_iterator vend() const { 
			return const_valid_iterator(theLastDate+1,theValues.end(),theValues.begin(),theValues.end()); }
		const_valid_iterator valid_iterator(const Date& d) const {
			int i = d-theFirstDate;
			return const_valid_iterator(d,theValues.begin()+i,theValues.begin(),theValues.end());
		}
		// data iterators
		const_data_iterator dbegin() const { return theValues.begin(); }
		const_data_iterator dend() const { return theValues.end(); }
		const_data_iterator data_iterator(const Date& d) const { return dbegin()+(d-theFirstDate); }
		// valid data iterators
		const_valid_data_iterator vdbegin() const { 
			return const_valid_data_iterator(theValues.begin(),theValues.begin(),theValues.end()); }
		const_valid_data_iterator vdend() const {
			return const_valid_data_iterator(theValues.end(),theValues.begin(),theValues.end()); }
		const_valid_data_iterator valid_data_iterator(const Date& d) const {
			int i = d-theFirstDate;
			return const_valid_data_iterator(theValues.begin()+i,theValues.begin(),theValues.end());
		}
		//@}
	  private:
		Date theFirstDate, theLastDate;
		std::vector<double> theValues;
	};


	// inline definitions

	inline History::History(const Date& firstDate, const Date& lastDate, const std::vector<double>& values)
	: theFirstDate(firstDate), theLastDate(lastDate), theValues(values) {
		Require(lastDate >= firstDate, "invalid date range for history");
		Require(values.size() == (lastDate-firstDate)+1, "history size incompatible with date range");
	}

	inline History::History(const std::vector<Date>& dates, const std::vector<double>& values) {
		Require(dates.size() == values.size(),"different size for date and value vectors");
		Require(dates.size() >= 1,"null history given");
		theFirstDate = theLastDate = dates[0];
		double lastValue = values[0];
		theValues = std::vector<double>(1,lastValue);
		for (int i=1; i<dates.size(); i++) {
			Date d = dates[i];
			double x = values[i];
			Require(d>=theLastDate,"unsorted date after "+DateFormatter::toString(theLastDate));
			if (d == theLastDate) {
				Require(x==lastValue,"different values in history for "+DateFormatter::toString(theLastDate));
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
