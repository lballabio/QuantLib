
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

/*! \file mathtools.h
	\brief Global include file for math-related classes.
*/

#include "qldefines.h"

#if QL_INCLUDE_IN_SUBFOLDERS
	#include "Math/normaldistribution.h"
	#include "Math/statistics.h"
	#include "Math/newcubicspline.h"
	#include "Math/location.h"		
	
#else
	#include "normaldistribution.h"
	#include "statistics.h"
	#include "newcubicspline.h"
	#include "location.h"		
#endif

