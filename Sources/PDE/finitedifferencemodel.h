
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_finite_difference_model_h
#define quantlib_finite_difference_model_h

#include "qldefines.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(PDE)

template<class Evolver>
class FiniteDifferenceModel {
  public:
	typedef typename Evolver::arrayType arrayType;
	typedef typename Evolver::operatorType operatorType;
	// constructor
	FiniteDifferenceModel(const operatorType& D) : evolver(D) {}
	// methods
	// arrayType grid() const { return evolver.xGrid(); }
	void rollback(arrayType& a, Time from, Time to, int steps);
  private:
	Evolver evolver;
};

// template definitions

template<class Evolver>
void FiniteDifferenceModel<Evolver>::rollback(FiniteDifferenceModel::arrayType& a, Time from, Time to, int steps) {
	// WARNING: it is a rollback: 'from' must be a later time than 'to'!
	Time dt = (from-to)/steps, t = from;
	evolver.setStep(dt);
	for (int i=0; i<steps; i++, t -= dt)
		evolver.step(a,t);
}

QL_END_NAMESPACE(PDE)

QL_END_NAMESPACE(QuantLib)


#endif
