
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_evolver_h
#define quantlib_evolver_h

#include "qldefines.h"

/*	Evolvers do not need to inherit from any base class.
	However, they must implement the following interface:

class Evolver {
  public:
	typedef ... arrayType;
	typedef ... operatorType;
	// constructors
	Evolver(const operatorType& D);
	// member functions
	void step(arrayType& a. Time t) const;	// The differential operator D could be time-dependent
	void setStep(Time dt);
};

*/


#endif
