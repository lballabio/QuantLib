
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

/*! \file finitedifferences.h
	\brief Global include file for the package on finite difference methods.
*/

#include "qldefines.h"

#if QL_INCLUDE_IN_SUBFOLDERS
	#include "FiniteDifferences/backwardeuler.h"
	#include "FiniteDifferences/bsmoperator.h"
	#include "FiniteDifferences/boundarycondition.h"
	#include "FiniteDifferences/cranknicolson.h"
	#include "FiniteDifferences/finitedifferencemodel.h"
	#include "FiniteDifferences/forwardeuler.h"
	#include "FiniteDifferences/identity.h"
	#include "FiniteDifferences/operator.h"
	#include "FiniteDifferences/operatortraits.h"
	#include "FiniteDifferences/stepcondition.h"
	#include "FiniteDifferences/tridiagonaloperator.h"
#else
	#include "backwardeuler.h"
	#include "bsmoperator.h"
	#include "boundarycondition.h"
	#include "cranknicolson.h"
	#include "finitedifferencemodel.h"
	#include "forwardeuler.h"
	#include "identity.h"
	#include "operator.h"
	#include "operatortraits.h"
	#include "stepcondition.h"
	#include "tridiagonaloperator.h"
#endif

