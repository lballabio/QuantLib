
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

#ifndef quantlib_h
#define quantlib_h

/*  This file includes all the QuantLib headers. Developers which want to link QuantLib
	to their own projects can simply include this file to have all QuantLib classes
	available.
	
	However, this file should not be used inside QuantLib itself. 
	Developers which want to extend QuantLib should include the specific files needed
	for their code.
	
	Even though this file includes all the QuantLib headers, it cannot set
	the include path for a compiler. The developer must take care himself of adding the
	QuantLib/Include directory and all its subdirectories to its include path.
*/

/*** global defines ***/

#include "qldefines.h"


/*** error checking - handling ***/

#include "qlerrors.h"


/*** financial types ***/

#include "discountfactor.h"
#include "spread.h"
#include "rate.h"


/*** date-related classes ***/

#include "date.h"
// day counters
#include "daycounter.h"
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
// calendars
#include "calendar.h"
#if QL_INCLUDE_IN_SUBFOLDERS
	#include "Calendars/westerncalendar.h"
	#include "Calendars/frankfurt.h"
	#include "Calendars/london.h"
	#include "Calendars/milan.h"
	#include "Calendars/newyork.h"
	#include "Calendars/target.h"
	#include "Calendars/zurich.h"
#else
	#include "westerncalendar.h"
	#include "frankfurt.h"
	#include "london.h"
	#include "milan.h"
	#include "newyork.h"
	#include "target.h"
	#include "zurich.h"
#endif

/*** currencies ***/

#include "currency.h"
#if QL_INCLUDE_IN_SUBFOLDERS
	#include "Currencies/dem.h"
	#include "Currencies/eur.h"
	#include "Currencies/gbp.h"
	#include "Currencies/itl.h"
	#include "Currencies/usd.h"
	#include "Currencies/aud.h"
	#include "Currencies/cad.h"
	#include "Currencies/chf.h"
	#include "Currencies/dkk.h"
	#include "Currencies/jpy.h"
	#include "Currencies/sek.h"
#else
	#include "dem.h"
	#include "eur.h"
	#include "gbp.h"
	#include "itl.h"
	#include "usd.h"
	#include "aud.h"
	#include "cad.h"
	#include "chf.h"
	#include "dkk.h"
	#include "jpy.h"
	#include "sek.h"
#endif


/*** instruments ***/

#include "instrument.h"
#if QL_INCLUDE_IN_SUBFOLDERS
	#include "Instruments/stock.h"
#else
	#include "stock.h"
#endif


/*** term structures ***/

#include "termstructure.h"
#if QL_INCLUDE_IN_SUBFOLDERS
	#include "TermStructures/piecewiseconstantforwards.h"
#else
	#include "piecewiseconstantforwards.h"
#endif


/*** volatility surfaces ***/

#include "swaptionvolsurface.h"
#include "forwardvolsurface.h"


/*** math ***/

#include "array.h"
// math tools
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
// finite difference methods
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
// 1-D solvers
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


/*** patterns ***/

#if QL_INCLUDE_IN_SUBFOLDERS
	#include "Patterns/observable.h"
#else
	#include "observable.h"
#endif


/*** pricers ***/

#include "options.h"
#if QL_INCLUDE_IN_SUBFOLDERS
	#include "Pricers/bsmoption.h"
	#include "Pricers/bsmeuropeanoption.h"
	#include "Pricers/bsmnumericaloption.h"
	#include "Pricers/americancondition.h"
	#include "Pricers/bsmamericanoption.h"
	#include "Pricers/dividendeuropeanoption.h"
	#include "Pricers/dividendamericanoption.h"
#else
	#include "bsmoption.h"
	#include "bsmeuropeanoption.h"
	#include "bsmnumericaloption.h"
	#include "americancondition.h"
	#include "bsmamericanoption.h"
	#include "dividendeuropeanoption.h"
	#include "dividendamericanoption.h"
#endif


/*** utilities ***/

#include "handle.h"
#include "null.h"
#include "dataformatters.h"


/*** shortcuts for the full namespaces - just to get us up and running quickly ***/

#define QL    QuantLib::
#define QLCAL QuantLib::Calendars::
#define QLCUR QuantLib::Currencies::
#define QLDCO QuantLib::DayCounters::
#define QLINS QuantLib::Instruments::
#define QLFDM QuantLib::FiniteDifferences::
#define QLMTH QuantLib::Math::
#define QLPAT QuantLib::Patterns::
#define QLPRC QuantLib::Pricers::
#define QLS1D QuantLib::Solvers1D::
#define QLTST QuantLib::TermStructures::


#endif
