
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

// $Source$

// $Log$
// Revision 1.3  2000/12/13 18:09:54  nando
// CVS keyword added
//

#ifndef quantlib_piecewise_constant_forward_curve_h
#define quantlib_piecewise_constant_forward_curve_h

#include "qldefines.h"
#include "termstructure.h"
#include "deposit.h"
#include <vector>

namespace QuantLib {

	namespace TermStructures {
	
		class PiecewiseConstantForwards : public TermStructure {
		  public:
			// constructor
			PiecewiseConstantForwards(Handle<Currency> currency, Handle<DayCounter> dayCounter, const Date& today, 
			  const std::vector<Deposit>& deposits);
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
			// methods
			int nextNode(const Date& d) const;
			// data members
			Handle<Currency> theCurrency;
			Handle<DayCounter> theDayCounter;
			Date today;
			std::vector<Date> theNodes;
			std::vector<Time> theTimes;
			std::vector<DiscountFactor> theDiscounts;
			std::vector<Rate> theForwards, theZeroYields;
			std::vector<Deposit> theDeposits;
		};
		
		// inline definitions
		
		inline Handle<TermStructure> PiecewiseConstantForwards::clone() const {
			return Handle<TermStructure>(new PiecewiseConstantForwards(theCurrency,theDayCounter,today,theDeposits));
		}
		
		inline Handle<Currency> PiecewiseConstantForwards::currency() const {
			return theCurrency;
		}
		
		inline Handle<DayCounter> PiecewiseConstantForwards::dayCounter() const {
			return theDayCounter;
		}
		
		inline Date PiecewiseConstantForwards::todaysDate() const {
			return today;
		}
		
		inline Date PiecewiseConstantForwards::settlementDate() const {
			return theCurrency->settlementDate(today);
		}
		
		inline Handle<Calendar> PiecewiseConstantForwards::calendar() const {
			return theCurrency->settlementCalendar();
		}
		
		inline Date PiecewiseConstantForwards::maxDate() const {
			return theNodes.back();
		}
		
		inline Date PiecewiseConstantForwards::minDate() const {
			return settlementDate();
		}
		
	}

}


#endif
