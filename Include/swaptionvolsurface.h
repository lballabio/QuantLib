
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
// Revision 1.6  2000/12/13 18:09:52  nando
// CVS keyword added
//

/*! \file swaptionvolsurface.h
	\brief Swaption volatility surface
*/

#ifndef quantlib_swaption_volatility_surface_h
#define quantlib_swaption_volatility_surface_h

#include "qldefines.h"
#include "date.h"
#include "rate.h"
#include "handle.h"
#include "observable.h"

namespace QuantLib {

	//! Swaption volatility surface
	/*! This class is purely abstract and defines the interface of concrete
		swaption volatility structures which will be derived from this one.
	*/
	class SwaptionVolatilitySurface : public Patterns::Observable {
	  public:
		virtual ~SwaptionVolatilitySurface() {}
		//! returns a copy of this surface with no observers registered
		virtual Handle<SwaptionVolatilitySurface> clone() const = 0;
		//! returns the volatility for a given starting date and length
		virtual Rate vol(const Date& start, Time length) const = 0;
	};
	
	//! Swaption volatility surface with an added spread
	/*! This surface will remain linked to the original surface, i.e., any changes
		in the latter will be reflected in this surface as well.
	*/
	class SpreadedSwaptionVolatilitySurface : public SwaptionVolatilitySurface {
	  public:
		SpreadedSwaptionVolatilitySurface(const Handle<SwaptionVolatilitySurface>&, Spread spread);
		Handle<SwaptionVolatilitySurface> clone() const;
		//! returns the volatility of the original surface plus the given spread
		Rate vol(const Date& start, Time length) const;
		//! registers with the original surface as well
		void registerObserver(Patterns::Observer*);
		//! unregisters with the original surface as well
		void unregisterObserver(Patterns::Observer*);
		//! unregisters with the original surface as well
		void unregisterAll();
	  private:
		Handle<SwaptionVolatilitySurface> theOriginalSurface;
		Spread theSpread;
	};
	
	
	// inline definitions
	
	inline SpreadedSwaptionVolatilitySurface::SpreadedSwaptionVolatilitySurface(
		const Handle<SwaptionVolatilitySurface>& h, Spread spread)
	: theOriginalSurface(h), theSpread(spread) {}
	
	inline Handle<SwaptionVolatilitySurface> SpreadedSwaptionVolatilitySurface::clone() const {
		return Handle<SwaptionVolatilitySurface>(new SpreadedSwaptionVolatilitySurface(
			theOriginalSurface->clone(),theSpread));
	}
	
	inline Rate SpreadedSwaptionVolatilitySurface::vol(const Date& start, Time length) const {
		return theOriginalSurface->vol(start,length)+theSpread;
	}
	
	inline void SpreadedSwaptionVolatilitySurface::registerObserver(Patterns::Observer* o) {
		SwaptionVolatilitySurface::registerObserver(o);
		theOriginalSurface->registerObserver(o);
	}
	
	inline void SpreadedSwaptionVolatilitySurface::unregisterObserver(Patterns::Observer* o) {
		SwaptionVolatilitySurface::unregisterObserver(o);
		theOriginalSurface->unregisterObserver(o);
	}
	
	inline void SpreadedSwaptionVolatilitySurface::unregisterAll() {
		for (std::set<Patterns::Observer*>::iterator i = observers().begin(); i!=observers().end(); ++i)
			theOriginalSurface->unregisterObserver(*i);
		SwaptionVolatilitySurface::unregisterAll();
	}

}


#endif
