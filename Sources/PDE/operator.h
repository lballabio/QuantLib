
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_operator_h
#define quantlib_operator_h

#include "qldefines.h"

namespace QuantLib {

	namespace PDE {
	
		class TimeConstantOperator {
		  public:
			enum { isTimeDependent = 0 };
		};
		
		class TimeDependentOperator {
		  public:
			enum { isTimeDependent = 1 };
		};
	
	}

}


#endif
