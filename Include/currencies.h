
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file currencies.h
	\brief Global include file for currency classes.
	
	This file includes the header files of all concrete currencies.
*/

#include "qldefines.h"

#if QL_INCLUDE_IN_SUBFOLDERS
	#include "Currencies/dem.h"
	#include "Currencies/eur.h"
	#include "Currencies/gbp.h"
	#include "Currencies/itl.h"
	#include "Currencies/usd.h"
#else
	#include "dem.h"
	#include "eur.h"
	#include "gbp.h"
	#include "itl.h"
	#include "usd.h"
#endif

