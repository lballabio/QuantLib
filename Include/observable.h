
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License: see the file LICENSE.TXT for details.
 * Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file.
 * LICENCE.TXT is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/


#ifndef quantlib_observable_h
#define quantlib_observable_h

#include "qldefines.h"
#include <set>

namespace QuantLib {

	namespace Patterns {
		
		// These classes are a simplified version of the ones implemented in Thinking in C++
		// which in turn mirror the Java Observer and Observable interface
		
		class Observer {
		  public:
			virtual void update() = 0;
		};
		
		class Observable {
		  public:
			virtual void registerObserver(Observer*);
			void registerObservers(std::set<Observer*>&);
			virtual void unregisterObserver(Observer*);
			void unregisterObservers(std::set<Observer*>&);
			virtual void unregisterAll();
			virtual void notifyObservers();
			std::set<Observer*> observers() const;
		  private:
			std::set<Observer*> theObservers;
		};
		
		
		// inline definitions
		
		inline void Observable::registerObserver(Observer* o) {
			theObservers.insert(o);
		}
		
		inline void Observable::registerObservers(std::set<Observer*>& o) {
			for (std::set<Observer*>::iterator i = o.begin(); i != o.end(); ++i)
				registerObserver(*i);
		}
		
		inline void Observable::unregisterObserver(Observer* o) {
			theObservers.erase(o);
		}
		
		inline void Observable::unregisterObservers(std::set<Observer*>& o) {
			for (std::set<Observer*>::iterator i = o.begin(); i != o.end(); ++i)
				unregisterObserver(*i);
		}
		
		inline void Observable::unregisterAll() {
			theObservers.clear();
		}
		
		inline void Observable::notifyObservers() {
			for (std::set<Observer*>::iterator i = theObservers.begin(); i != theObservers.end(); ++i)
				(*i)->update();
		}
		
		inline std::set<Observer*> Observable::observers() const {
			return theObservers;
		}

	}

}


#endif
