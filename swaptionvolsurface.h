
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_swaption_volatility_surface_h
#define quantlib_swaption_volatility_surface_h

#include "qldefines.h"
#include "date.h"
#include "yield.h"
#include "handle.h"
#include "observable.h"

QL_BEGIN_NAMESPACE(QuantLib)

class SwaptionVolatilitySurface : public Observable {
  public:
	// constructors
	SwaptionVolatilitySurface() {};
	// copy of this surface with no observers registered
	virtual Handle<SwaptionVolatilitySurface> clone() const = 0;
	// volatility
	virtual Yield vol(const Date& start, Time length) const = 0;
};

// spreaded surface 

class SpreadedSwaptionVolatilitySurface : public SwaptionVolatilitySurface {
  public:
	// constructor
	SpreadedSwaptionVolatilitySurface(Handle<SwaptionVolatilitySurface>, Spread spread);
	// clone
	Handle<SwaptionVolatilitySurface> clone() const;
	// volatility
	Yield vol(const Date& start, Time length) const;
	// observers of this curve are also observers of the original curve
	void registerObserver(Observer*);
	void unregisterObserver(Observer*);
	void unregisterAll();
  private:
	Handle<SwaptionVolatilitySurface> theOriginalSurface;
	Spread theSpread;
};


// inline definitions

inline SpreadedSwaptionVolatilitySurface::SpreadedSwaptionVolatilitySurface(
	Handle<SwaptionVolatilitySurface> h, Spread spread)
: theOriginalSurface(h), theSpread(spread) {}

inline Handle<SwaptionVolatilitySurface> SpreadedSwaptionVolatilitySurface::clone() const {
	return Handle<SwaptionVolatilitySurface>(new SpreadedSwaptionVolatilitySurface(
		theOriginalSurface->clone(),theSpread));
}

inline Yield SpreadedSwaptionVolatilitySurface::vol(const Date& start, Time length) const {
	return theOriginalSurface->vol(start,length)+theSpread;
}

inline void SpreadedSwaptionVolatilitySurface::registerObserver(Observer* o) {
	SwaptionVolatilitySurface::registerObserver(o);
	theOriginalSurface->registerObserver(o);
}

inline void SpreadedSwaptionVolatilitySurface::unregisterObserver(Observer* o) {
	SwaptionVolatilitySurface::unregisterObserver(o);
	theOriginalSurface->unregisterObserver(o);
}

inline void SpreadedSwaptionVolatilitySurface::unregisterAll() {
	for (std::set<Observer*>::iterator i = observers().begin(); i!=observers().end(); ++i)
		theOriginalSurface->unregisterObserver(*i);
	SwaptionVolatilitySurface::unregisterAll();
}

QL_END_NAMESPACE(QuantLib)


#endif
