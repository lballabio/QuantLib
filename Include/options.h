
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
