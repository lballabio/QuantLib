
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_observable_h
#define quantlib_observable_h

#include "qldefines.h"
#include <set>

QL_BEGIN_NAMESPACE(QuantLib)

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

QL_END_NAMESPACE(QuantLib)


#endif
