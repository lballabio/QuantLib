
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_forward_volatility_surface_h
#define quantlib_forward_volatility_surface_h

#include "qldefines.h"
#include "date.h"
#include "yield.h"
#include "handle.h"
#include "observable.h"

namespace QuantLib {

	class ForwardVolatilitySurface : public Observable {
	  public:
		// constructors
		ForwardVolatilitySurface() {};
		// copy of this curve with no observers registered
		virtual Handle<ForwardVolatilitySurface> clone() const = 0;
		// volatility
		virtual Rate vol(const Date& d, Rate strike) const = 0;
	};
	
	// curve, i.e., no smile
	
	class ForwardVolatilityCurve : public ForwardVolatilitySurface {
	  public:
		// constructors
		ForwardVolatilityCurve() {};
		// copy of this curve with no observers registered
		virtual Handle<ForwardVolatilitySurface> clone() const = 0;
		// volatility
		virtual Rate vol(const Date& d, Rate strike) const;
		// independent of strike
		virtual Rate vol(const Date& d) const = 0;
	};
	
	class ConstantForwardVolatilitySurface : public ForwardVolatilitySurface {
	  public:
		// constructors
		ConstantForwardVolatilitySurface(Rate volatility);
		// copy of this curve with no observers registered
		Handle<ForwardVolatilitySurface> clone() const;
		// volatility
		Rate vol(const Date& d, Rate strike) const;
	  private:
		Rate theVolatility;
	};
	
	class SpreadedForwardVolatilitySurface : public ForwardVolatilitySurface {
	  public:
		// constructor
		SpreadedForwardVolatilitySurface(const Handle<ForwardVolatilitySurface>&, Spread spread);
		// clone
		Handle<ForwardVolatilitySurface> clone() const;
		// volatility
		Rate vol(const Date& d, Rate strike) const;
		// observers of this curve are also observers of the original curve
		void registerObserver(Observer*);
		void unregisterObserver(Observer*);
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
	
	inline void SpreadedForwardVolatilitySurface::registerObserver(Observer* o) {
		ForwardVolatilitySurface::registerObserver(o);
		theOriginalCurve->registerObserver(o);
	}
	
	inline void SpreadedForwardVolatilitySurface::unregisterObserver(Observer* o) {
		ForwardVolatilitySurface::unregisterObserver(o);
		theOriginalCurve->unregisterObserver(o);
	}
	
	inline void SpreadedForwardVolatilitySurface::unregisterAll() {
		for (std::set<Observer*>::iterator i = observers().begin(); i!=observers().end(); ++i)
			theOriginalCurve->unregisterObserver(*i);
		ForwardVolatilitySurface::unregisterAll();
	}

}


#endif
