
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

/*! \file flatforward
	\brief flat forward rate term structure
	
	$Source$
	$Name$
	$Log$
	Revision 1.2  2000/12/14 12:32:31  lballabio
	Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_flat_forward_curve_h
#define quantlib_flat_forward_curve_h

#include "qldefines.h"
#include "termstructure.h"

namespace QuantLib {

	namespace TermStructures {
	
		class FlatForward : public TermStructure {
		  public:
			// constructor
			FlatForward(const Handle<Currency>& currency, const Handle<DayCounter>& dayCounter,
			  const Date& today, Rate forward);
			// clone
			Handle<TermStructure> clone() const;
			// inspectors
			Handle<Currency> currency() const;
			Handle<DayCounter> dayCounter() const;
			Date todaysDate() const;
			Date settlementDate() const;
			Handle<Calendar> calendar() const;
			Date maxDate() const;
			Date minDate() const;
			// zero yield
			Rate zeroYield(const Date&) const;
			// discount
			DiscountFactor discount(const Date&) const;
			// forward (instantaneous)
			Rate forward(const Date&) const;
		  private:
			Handle<Currency> theCurrency;
			Handle<DayCounter> theDayCounter;
			Date today;
			Rate theForward;
		};
		
		// inline definitions
		
		inline FlatForward::FlatForward(const Handle<Currency>& currency, const Handle<DayCounter>& dayCounter,
		  const Date& today, Rate forward)
		: theCurrency(currency), theDayCounter(dayCounter), today(today), theForward(forward) {}
		
		inline Handle<TermStructure> FlatForward::clone() const {
			return Handle<TermStructure>(new FlatForward(theCurrency,theDayCounter,today,theForward));
		}
		
		inline Handle<Currency> FlatForward::currency() const {
			return theCurrency;
		}
		
		inline Handle<DayCounter> FlatForward::dayCounter() const {
			return theDayCounter;
		}
		
		inline Date FlatForward::todaysDate() const {
			return today;
		}
		
		inline Date FlatForward::settlementDate() const {
			return theCurrency->settlementDate(today);
		}
		
		inline Handle<Calendar> FlatForward::calendar() const {
			return theCurrency->settlementCalendar();
		}
		
		inline Date FlatForward::maxDate() const {
			return Date::maxDate();
		}
		
		inline Date FlatForward::minDate() const {
			return settlementDate();
		}
		
		inline Rate FlatForward::zeroYield(const Date& d) const {
			Require(d>=minDate() && d<=maxDate(), "date outside curve definition");
			return theForward;
		}
		
		inline DiscountFactor FlatForward::discount(const Date& d) const {
			Require(d>=minDate() && d<=maxDate(), "date outside curve definition");
			double t = theDayCounter->yearFraction(settlementDate(),d);
			return DiscountFactor(QL_EXP(-theForward*t));
		}
		
		inline Rate FlatForward::forward(const Date& d) const {
			Require(d>=minDate() && d<=maxDate(), "date outside curve definition");
			return theForward;
		}

	}

}


#endif
