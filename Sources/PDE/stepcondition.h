
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_step_condition_h
#define quantlib_step_condition_h

#include "qldefines.h"
#include "date.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(PDE)

template <class arrayType>
class StepCondition {
  public:
	virtual void applyTo(arrayType& a, Time t) const = 0;
};


QL_END_NAMESPACE(PDE)

QL_END_NAMESPACE(QuantLib)


#endif
