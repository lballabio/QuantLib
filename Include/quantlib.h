
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
#include "actual360.h"
#include "actual365.h"
#include "actualactual.h"
#include "thirty360.h"
#include "thirty360european.h"
#include "thirty360italian.h"

// calendars
#include "calendar.h"
#include "westerncalendar.h"
#include "frankfurt.h"
#include "london.h"
#include "milan.h"
#include "newyork.h"
#include "target.h"
#include "zurich.h"


/*** currencies ***/

#include "currency.h"
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


/*** instruments ***/

#include "instrument.h"
#include "stock.h"


/*** term structures ***/

#include "termstructure.h"
#include "piecewiseconstantforwards.h"
#include "flatforward.h"


/*** volatility surfaces ***/

#include "swaptionvolsurface.h"
#include "forwardvolsurface.h"


/*** math ***/

#include "array.h"

// math tools
#include "normaldistribution.h"
#include "statistics.h"
#include "newcubicspline.h"
#include "location.h"		

// finite difference methods
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

// 1-D solvers
#include "solver1d.h"
#include "bisection.h"
#include "brent.h"
#include "falseposition.h"
#include "newton.h"
#include "newtonsafe.h"
#include "ridder.h"
#include "secant.h"


/*** patterns ***/

#include "observable.h"


/*** pricers ***/

#include "options.h"
#include "bsmoption.h"
#include "bsmeuropeanoption.h"
#include "bsmnumericaloption.h"
#include "americancondition.h"
#include "bsmamericanoption.h"
#include "dividendeuropeanoption.h"
#include "dividendamericanoption.h"


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
