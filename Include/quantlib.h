
/*
 * Copyright (C) 2000, 2001
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
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*
    $Source$
    $Name$
    $Log$
    Revision 1.26  2001/03/07 17:18:29  marmar
    Example of european option using finite differences

    Revision 1.25  2001/03/06 16:59:52  marmar
    First, simplified version, of everest option

    Revision 1.24  2001/03/02 08:36:44  enri
    Shout options added:
    	* BSMAmericanOption is now AmericanOption, same interface
    	* ShoutOption added
    	* both ShoutOption and AmericanOption inherit from
    	  StepConditionOption
    offline.doxy.linux added.

    Revision 1.23  2001/03/01 12:57:37  enri
    class BinaryOption added, test it with binary_option.py

    Revision 1.22  2001/02/20 11:11:37  marmar
    BarrierOption implements the analytical barrier option

    Revision 1.21  2001/02/14 18:43:06  lballabio
    Added coupling iterators

    Revision 1.20  2001/01/25 11:57:32  lballabio
    Included outer product and sqrt into matrix.h

    Revision 1.19  2001/01/25 10:31:22  marmar
    JacobiDecomposition renamed SymmetricSchurDecomposition

    Revision 1.18  2001/01/24 16:02:56  marmar
    math files added

    Revision 1.17  2001/01/23 18:12:50  lballabio
    Added matrix.h to Include/Math

*/

#ifndef quantlib_h
#define quantlib_h


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
#include "matrix.h"
#include "symmetricschurdecomposition.h"
#include "multivariateaccumulator.h"

// math tools
#include "normaldistribution.h"
#include "statistics.h"
#include "interpolation.h"
#include "linearinterpolation.h"
#include "cubicspline.h"
#include "lexicographicalview.h"

// Monte Carlo tools
#include "boxmuller.h"
#include "centrallimitgaussian.h"
#include "gaussianarraygenerator.h"
#include "gaussianrandomgenerator.h"
#include "lecuyerrandomgenerator.h"
#include "uniformrandomgenerator.h"
#include "standardmultipathgenerator.h"
#include "standardpathgenerator.h"

// finite difference tools
#include "backwardeuler.h"
#include "bsmoperator.h"
#include "boundarycondition.h"
#include "cranknicolson.h"
#include "dminus.h"
#include "dplus.h"
#include "dplusdminus.h"
#include "dzero.h"
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

#include "americancondition.h"
#include "americanoption.h"
#include "averagepriceasian.h"
#include "averagestrikeasian.h"
#include "binaryoption.h"
#include "barrieroption.h"
#include "bsmoption.h"
#include "bsmeuropeanoption.h"
#include "bsmnumericaloption.h"
#include "dividendeuropeanoption.h"
#include "dividendamericanoption.h"
#include "everestoption.h"
#include "finitedifferenceeuropean.h"
#include "geometricasianoption.h"
#include "himalaya.h"
#include "mceuropeanpricer.h"
#include "options.h"
#include "plainbasketoption.h"
#include "shoutcondition.h"
#include "stepconditionoption.h"
#include "shoutoption.h"



/*** utilities ***/

#include "handle.h"
#include "null.h"
#include "dataformatters.h"
#include "combiningiterator.h"
#include "couplingiterator.h"
#include "steppingiterator.h"
#include "filteringiterator.h"
#include "processingiterator.h"
#include "iteratorcategories.h"


/*** shortcuts for the full namespaces
     - just to get us up and running quickly ***/

#define QL    QuantLib::
#define QLCAL QuantLib::Calendars::
#define QLCUR QuantLib::Currencies::
#define QLDCO QuantLib::DayCounters::
#define QLINS QuantLib::Instruments::
#define QLFDM QuantLib::FiniteDifferences::
#define QLMTH QuantLib::Math::
#define QLMNT QuantLib::MonteCarlo::
#define QLPAT QuantLib::Patterns::
#define QLPRC QuantLib::Pricers::
#define QLS1D QuantLib::Solvers1D::
#define QLTST QuantLib::TermStructures::
#define QLUTL QuantLib::Utilities::


#endif
