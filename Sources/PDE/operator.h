
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_operator_h
#define quantlib_operator_h

#include "qldefines.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(PDE)

class TimeConstantOperator {
  public:
	enum { isTimeDependent = 0 };
};

class TimeDependentOperator {
  public:
	enum { isTimeDependent = 1 };
};

QL_END_NAMESPACE(PDE)

QL_END_NAMESPACE(QuantLib)


#endif
