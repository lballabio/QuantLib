
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


#ifndef quantlib_finite_difference_model_h
#define quantlib_finite_difference_model_h

#include "qldefines.h"
#include "stepcondition.h"
#include "handle.h"
#include "null.h"

namespace QuantLib {

	namespace PDE {
	
		template<class Evolver>
		class FiniteDifferenceModel {
		  public:
			typedef typename Evolver::arrayType arrayType;
			typedef typename Evolver::operatorType operatorType;
			// constructor
			FiniteDifferenceModel(const operatorType& D) : evolver(D) {}
			// methods
			// arrayType grid() const { return evolver.xGrid(); }
			void rollback(arrayType& a, Time from, Time to, int steps, 
			  Handle<StepCondition<arrayType> > condition = Handle<StepCondition<arrayType> >());
		  private:
			Evolver evolver;
		};
		
		// template definitions
		
		template<class Evolver>
		void FiniteDifferenceModel<Evolver>::rollback(FiniteDifferenceModel::arrayType& a, Time from, Time to, int steps,
		  Handle<StepCondition<arrayType> > condition) {
			// WARNING: it is a rollback: 'from' must be a later time than 'to'!
			Time dt = (from-to)/steps, t = from;
			evolver.setStep(dt);
			for (int i=0; i<steps; i++, t -= dt) {
				evolver.step(a,t);
				if (!IsNull(condition))
					condition->applyTo(a,t);
			}
		}
	
	}

}


#endif
