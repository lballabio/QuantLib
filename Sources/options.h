
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

/*! \file options.h
	\brief Base option class
*/

#ifndef quantlib_options_h
#define quantlib_options_h

#include "qldefines.h"

namespace QuantLib {

	//! base option class
	/*! This class has the only purpose of encapsulating the option type enumeration.
	*/
	class Option {
	  public:
		enum Type { Call, Put, Straddle };
	};

}


#endif
