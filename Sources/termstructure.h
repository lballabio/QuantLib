
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_term_structure_h
#define quantlib_term_structure_h

#include "qldefines.h"
#include "date.h"
#include "calendar.h"
#include "yield.h"
#include "spread.h"
#include "discountfactor.h"
#include "currency.h"
#include "handle.h"
#include "observable.h"
#include <vector>

namespace QuantLib {

	class TermStructure : public Observable {
	  public:
		// constructor
		TermStructure() {}
		virtual ~TermStructure() {}
		// copy of this curve with no observers registered
		virtual Handle<TermStructure> clone() const = 0;
		// inspectors
		virtual Handle<Currency> currency() const = 0;
		virtual Date todaysDate() const = 0;
		virtual Date settlementDate() const = 0;
		virtual Handle<Calendar> calendar() const = 0;
		virtual Date maxDate() const = 0;
		virtual Date minDate() const = 0;
		// zero yield
		virtual Rate zeroYield(const Date&) const = 0;
		std::vector<Rate> zeroYield(const std::vector<Date>&) const;
		// discount
		virtual DiscountFactor discount(const Date&) const = 0;
		std::vector<DiscountFactor> discount(const std::vector<Date>&) const;
		// forward
		virtual Rate forward(const Date&) const = 0;
		std::vector<Rate> forward(const std::vector<Date>&) const;
	};
	
	class ZeroYieldStructure : public TermStructure {
	  public:
		// constructor
		ZeroYieldStructure() {}
		virtual ~ZeroYieldStructure() {}
		DiscountFactor discount(const Date&) const;
		Rate forward(const Date&) const;
	};
	
	class DiscountStructure : public TermStructure {
	  public:
		// constructor
		DiscountStructure() {}
		virtual ~DiscountStructure() {}
		Rate zeroYield(const Date&) const;
		Rate forward(const Date&) const;
	};
	
	class ForwardRateStructure : public TermStructure {
	  public:
		// constructor
		ForwardRateStructure() {}
		virtual ~ForwardRateStructure() {}
		Rate zeroYield(const Date&) const;
		DiscountFactor discount(const Date&) const;
	};
	
	// note: the shifted and spreaded curves remain linked to the original curve
	
	class ImpliedTermStructure : public DiscountStructure {
	  public:
		// constructor
		ImpliedTermStructure(const Handle<TermStructure>&, const Date& evaluationDate);
		// clone
		Handle<TermStructure> clone() const;
		// inspectors
		Handle<Currency> currency() const;
		Date todaysDate() const;
		Date settlementDate() const;
		Handle<Calendar> calendar() const;
		Date maxDate() const;
		Date minDate() const;
		// discount
		DiscountFactor discount(const Date&) const;
		// observers of this curve are also observers of the original curve
		void registerObserver(Observer*);
		void unregisterObserver(Observer*);
		void unregisterAll();
	  private:
		Handle<TermStructure> theOriginalCurve;
		Date theEvaluationDate;
	};
	
	class SpreadedTermStructure : public ZeroYieldStructure {
	  public:
		// constructor
		SpreadedTermStructure(const Handle<TermStructure>&, Spread spread);
		// clone
		Handle<TermStructure> clone() const;
		// inspectors
		Handle<Currency> currency() const;
		Date todaysDate() const;
		Date settlementDate() const;
		Handle<Calendar> calendar() const;
		Date maxDate() const;
		Date minDate() const;
		// discount
		Rate zeroYield(const Date&) const;
		// observers of this curve are actually observers of the original curve
		void registerObserver(Observer*);
		void unregisterObserver(Observer*);
		void unregisterAll();
	  private:
		Handle<TermStructure> theOriginalCurve;
		Spread theSpread;
	};
	
	
	// inline definitions
	
	inline std::vector<Rate> TermStructure::zeroYield(const std::vector<Date>& x) const {
		std::vector<Rate> y(x.size());
		std::vector<Date>::const_iterator j=x.begin();
		for (std::vector<Rate>::iterator i=y.begin(); i!=y.end(); ++i,++j)
			*i = zeroYield(*j);
		return y;
	}
	
	inline std::vector<DiscountFactor> TermStructure::discount(const std::vector<Date>& x) const {
		std::vector<DiscountFactor> y(x.size());
		std::vector<Date>::const_iterator j=x.begin();
		for (std::vector<DiscountFactor>::iterator i=y.begin(); i!=y.end(); ++i,++j)
			*i = discount(*j);
		return y;
	}
	
	inline std::vector<Rate> TermStructure::forward(const std::vector<Date>& x) const {
		std::vector<Rate> y(x.size());
		std::vector<Date>::const_iterator j=x.begin();
		for (std::vector<Rate>::iterator i=y.begin(); i!=y.end(); ++i,++j)
			*i = forward(*j);
		return y;
	}
	
	
	// curve deriving discount and forward from zero yield
	
	inline DiscountFactor ZeroYieldStructure::discount(const Date& d) const {
		Rate r = zeroYield(d);
		double t = double(d-settlementDate())/365;
		return DiscountFactor(QL_EXP(-r*t));
	}
	
	inline Rate ZeroYieldStructure::forward(const Date& d) const {
		Rate r1 = zeroYield(d), r2 = zeroYield(d+1);
		return r1+(d-settlementDate())*double(r2-r1);			// r1+t*(r2-r1)/dt = r1+(days/365)*(r2-r1)/(1 day/365)
	}
	
	
	// curve deriving zero yield and forward from discount
	
	inline Rate DiscountStructure::zeroYield(const Date& d) const {
		DiscountFactor f = discount(d);
		double t = double(d-settlementDate())/365;
		return Rate(-QL_LOG(f)/t);
	}
	
	inline Rate DiscountStructure::forward(const Date& d) const {
		DiscountFactor f1 = discount(d), f2 = discount(d+1);
		return Rate(QL_LOG(f1/f2)*365);					// log(f1/f2)/dt = log(f1/f2)/(1/365)
	}
	
	
	// curve deriving zero yield and discount from forward
	
	inline Rate ForwardRateStructure::zeroYield(const Date& d) const {
		// This is just a default, highly inefficient implementation.
		// Derived classes should implement their own zeroYield method.
		if (d == settlementDate())
			return forward(settlementDate());
		double sum = 0.5*forward(settlementDate());
		for (Date i=settlementDate()+1; i<d; i++)
			sum += forward(i);
		sum += 0.5*forward(d);
		return Rate(sum/(d-settlementDate()));
	}
	
	inline DiscountFactor ForwardRateStructure::discount(const Date& d) const {
		Rate r = zeroYield(d);
		double t = double(d-settlementDate())/365;
		return DiscountFactor(QL_EXP(-r*t));
	}
	
	
	// time-shifted curve
	
	inline ImpliedTermStructure::ImpliedTermStructure(const Handle<TermStructure>& h, const Date& evaluationDate)
	: theOriginalCurve(h), theEvaluationDate(evaluationDate) {}
	
	inline Handle<Currency> ImpliedTermStructure::currency() const {
		return theOriginalCurve->currency();
	}
	
	inline Date ImpliedTermStructure::todaysDate() const {
		return theEvaluationDate;
	}
	
	inline Date ImpliedTermStructure::settlementDate() const {
		return theOriginalCurve->currency()->settlementDate(theEvaluationDate);
	}
	
	inline Handle<Calendar> ImpliedTermStructure::calendar() const {
		return theOriginalCurve->calendar();
	}
	
	inline Date ImpliedTermStructure::maxDate() const {
		return theOriginalCurve->maxDate();
	}
	
	inline Date ImpliedTermStructure::minDate() const {
		return settlementDate();
	}
	
	inline DiscountFactor ImpliedTermStructure::discount(const Date& d) const {
		return theOriginalCurve->discount(d)/theOriginalCurve->discount(theEvaluationDate);
	}
	
	inline Handle<TermStructure> ImpliedTermStructure::clone() const {
		return Handle<TermStructure>(new ImpliedTermStructure(theOriginalCurve->clone(),theEvaluationDate));
	}
	
	inline void ImpliedTermStructure::registerObserver(Observer* o) {
		TermStructure::registerObserver(o);
		theOriginalCurve->registerObserver(o);
	}
	
	inline void ImpliedTermStructure::unregisterObserver(Observer* o) {
		TermStructure::unregisterObserver(o);
		theOriginalCurve->unregisterObserver(o);
	}
	
	inline void ImpliedTermStructure::unregisterAll() {
		for (std::set<Observer*>::iterator i = observers().begin(); i!=observers().end(); ++i)
			theOriginalCurve->unregisterObserver(*i);
		TermStructure::unregisterAll();
	}
	
	
	// spreaded curve
	
	inline SpreadedTermStructure::SpreadedTermStructure(const Handle<TermStructure>& h, Spread spread)
	: theOriginalCurve(h), theSpread(spread) {}
	
	inline Handle<Currency> SpreadedTermStructure::currency() const {
		return theOriginalCurve->currency();
	}
	
	inline Date SpreadedTermStructure::todaysDate() const {
		return theOriginalCurve->todaysDate();
	}
	
	inline Date SpreadedTermStructure::settlementDate() const {
		return theOriginalCurve->settlementDate();
	}
	
	inline Handle<Calendar> SpreadedTermStructure::calendar() const {
		return theOriginalCurve->calendar();
	}
	
	inline Date SpreadedTermStructure::maxDate() const {
		return theOriginalCurve->maxDate();
	}
	
	inline Date SpreadedTermStructure::minDate() const {
		return theOriginalCurve->minDate();
	}
	
	inline Rate SpreadedTermStructure::zeroYield(const Date& d) const {
		return theOriginalCurve->zeroYield(d)+theSpread;
	}
	
	inline Handle<TermStructure> SpreadedTermStructure::clone() const {
		return Handle<TermStructure>(new SpreadedTermStructure(theOriginalCurve->clone(),theSpread));
	}
	
	inline void SpreadedTermStructure::registerObserver(Observer* o) {
		TermStructure::registerObserver(o);
		theOriginalCurve->registerObserver(o);
	}
	
	inline void SpreadedTermStructure::unregisterObserver(Observer* o) {
		TermStructure::unregisterObserver(o);
		theOriginalCurve->unregisterObserver(o);
	}
	
	inline void SpreadedTermStructure::unregisterAll() {
		for (std::set<Observer*>::iterator i = observers().begin(); i!=observers().end(); ++i)
			theOriginalCurve->unregisterObserver(*i);
		TermStructure::unregisterAll();
	}

}

#endif
