
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_options_h
#define quantlib_options_h

#include "qldefines.h"

QL_BEGIN_NAMESPACE(QuantLib)

class Option {
  public:
	enum Type { Call, Put, Straddle };
};

QL_END_NAMESPACE(QuantLib)


#endif
