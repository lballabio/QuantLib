
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

/*! \file forwardvolsurface.h
	\brief Forward volatility surface
	
	$Source$
	$Name$
	$Log$
	Revision 1.10  2000/12/27 14:05:56  lballabio
	Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

	Revision 1.9  2000/12/22 15:13:45  aleppo
	Included spread.h
	
	Revision 1.8  2000/12/14 12:32:29  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

#ifndef quantlib_forward_volatility_surface_h
#define quantlib_forward_volatility_surface_h

#include "qldefines.h"
#include "date.h"
#include "rate.h"
#include "spread.h"
#include "handle.h"
#include "observable.h"
#include "spread.h"

namespace QuantLib {

	//! Forward volatility surface
	/*! This class is purely abstract and defines the interface of concrete
		forward volatility structures which will be derived from this one.
				
		\todo A way should be implemented of constructing a ForwardVolatilitySurface
		from a generic term volatility structure.
	*/
	class ForwardVolatilitySurface : public Patterns::Observable {
	  public:
		virtual ~ForwardVolatilitySurface() {}
		//! returns a copy of this surface with no observers registered
		virtual Handle<ForwardVolatilitySurface> clone() const = 0;
		//! returns the volatility for a given date and strike
		virtual Rate vol(const Date& d, Rate strike) const = 0;
	};
	
	//! Forward volatility curve (not smiled)
	class ForwardVolatilityCurve : public ForwardVolatilitySurface {
	  public:
		virtual Handle<ForwardVolatilitySurface> clone() const = 0;
		//! implementation of the ForwardVolatilitySurface interface.
		/*! This method calls vol(date) to return the volatility for a given date regardless of strike */
		virtual Rate vol(const Date& d, Rate strike) const;
		//! returns the volatility for a given date
		/*! This method must be implemented by derived classes instead of vol(date,strike) which
			calls this one after discarding the strike */
		virtual Rate vol(const Date& d) const = 0;
	};
	
	//! Constant forward volatility surface
	class ConstantForwardVolatilitySurface : public ForwardVolatilitySurface {
	  public:
		ConstantForwardVolatilitySurface(Rate volatility);
		Handle<ForwardVolatilitySurface> clone() const;
		//! returns the given volatility regardless of date and strike
		Rate vol(const Date& d, Rate strike) const;
	  private:
		Rate theVolatility;
	};
	
	//! Forward volatility surface with an added spread
	/*! This surface will remain linked to the original surface, i.e., any changes
		in the latter will be reflected in this surface as well.
	*/
	class SpreadedForwardVolatilitySurface : public ForwardVolatilitySurface {
	  public:
		SpreadedForwardVolatilitySurface(const Handle<ForwardVolatilitySurface>&, Spread spread);
		Handle<ForwardVolatilitySurface> clone() const;
		//! returns the volatility of the original surface plus the given spread
		Rate vol(const Date& d, Rate strike) const;
		//! registers with the original surface as well
		void registerObserver(Patterns::Observer*);
		//! unregisters with the original surface as well
		void unregisterObserver(Patterns::Observer*);
		//! unregisters with the original surface as well
		void unregisterAll();
	  private:
		Handle<ForwardVolatilitySurface> theOriginalCurve;
		Spread theSpread;
	};
	
	
	// inline definitions
	
	// curve without smile
	
	inline Rate ForwardVolatilityCurve::vol(const Date& d, Rate strike) const {
		return vol(d);
	}
	
	// constant surface
	
	inline ConstantForwardVolatilitySurface::ConstantForwardVolatilitySurface(Rate volatility)
	: theVolatility(volatility) {}
	
	inline Handle<ForwardVolatilitySurface> ConstantForwardVolatilitySurface::clone() const {
		return Handle<ForwardVolatilitySurface>(new ConstantForwardVolatilitySurface(theVolatility));
	}
	
	inline Rate ConstantForwardVolatilitySurface::vol(const Date& d, Rate strike) const {
		return theVolatility;
	}
	
	// spreaded surface
	
	inline SpreadedForwardVolatilitySurface::SpreadedForwardVolatilitySurface(
		const Handle<ForwardVolatilitySurface>& h, Spread spread)
	: theOriginalCurve(h), theSpread(spread) {}
	
	inline Handle<ForwardVolatilitySurface> SpreadedForwardVolatilitySurface::clone() const {
		return Handle<ForwardVolatilitySurface>(new SpreadedForwardVolatilitySurface(
			theOriginalCurve->clone(),theSpread));
	}
	
	inline Rate SpreadedForwardVolatilitySurface::vol(const Date& d, Rate strike) const {
		return theOriginalCurve->vol(d,strike)+theSpread;
	}
	
	inline void SpreadedForwardVolatilitySurface::registerObserver(Patterns::Observer* o) {
		ForwardVolatilitySurface::registerObserver(o);
		theOriginalCurve->registerObserver(o);
	}
	
	inline void SpreadedForwardVolatilitySurface::unregisterObserver(Patterns::Observer* o) {
		ForwardVolatilitySurface::unregisterObserver(o);
		theOriginalCurve->unregisterObserver(o);
	}
	
	inline void SpreadedForwardVolatilitySurface::unregisterAll() {
		for (std::set<Patterns::Observer*>::iterator i = observers().begin(); i!=observers().end(); ++i)
			theOriginalCurve->unregisterObserver(*i);
		ForwardVolatilitySurface::unregisterAll();
	}

}


#endif
