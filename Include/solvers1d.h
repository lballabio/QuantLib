
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

/*! \file solvers1d.h
	\brief Global include file for 1-D solver classes.
	
	This file includes the header files of all concrete 1-D solvers.
*/

#include "qldefines.h"

#if QL_INCLUDE_IN_SUBFOLDERS
	#include "Solvers1D/bisection.h"
	#include "Solvers1D/brent.h"
	#include "Solvers1D/falseposition.h"
	#include "Solvers1D/newton.h"
	#include "Solvers1D/newtonsafe.h"
	#include "Solvers1D/ridder.h"
	#include "Solvers1D/secant.h"
#else
	#include "bisection.h"
	#include "brent.h"
	#include "falseposition.h"
	#include "newton.h"
	#include "newtonsafe.h"
	#include "ridder.h"
	#include "secant.h"
#endif

