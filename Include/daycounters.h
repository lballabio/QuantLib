
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

#include "qldefines.h"

#if QL_INCLUDE_IN_SUBFOLDERS
	#include "DayCounters/actual360.h"
	#include "DayCounters/actual365.h"
	#include "DayCounters/actualactual.h"
	#include "DayCounters/thirty360.h"
	#include "DayCounters/thirty360european.h"
	#include "DayCounters/thirty360italian.h"
#else
	#include "actual360.h"
	#include "actual365.h"
	#include "actualactual.h"
	#include "thirty360.h"
	#include "thirty360european.h"
	#include "thirty360italian.h"
#endif

