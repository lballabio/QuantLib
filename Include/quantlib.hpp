
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
    Revision 1.1  2001/04/04 11:07:21  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.34  2001/03/28 07:37:20  marmar
    file added

    Revision 1.33  2001/03/26 09:59:33  lballabio
    Added Helsinki and Wellington calendars

    Revision 1.32  2001/03/22 12:24:27  marmar
    Introducing pagoda options

    Revision 1.31  2001/03/21 10:49:58  marmar
    valueAtCenter, firstDerivativeAtCenter, secondDerivativeAtCenter,
    are no longer methods of BSMNumericalOption but separate
    functions

    Revision 1.30  2001/03/21 09:58:26  marmar
    BermudanOption file added

    Revision 1.29  2001/03/20 15:14:33  marmar
    MultiPeriodOption is a generalization of DividendAmericanOption

    Revision 1.28  2001/03/15 13:49:01  marmar
    getCovariance function added

    Revision 1.27  2001/03/09 12:40:39  lballabio
    Spring cleaning for SWIG interfaces

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

#include "qldefines.hpp"


/*** error checking - handling ***/

#include "qlerrors.hpp"


/*** financial types ***/

#include "discountfactor.hpp"
#include "spread.hpp"
#include "rate.hpp"


/*** date-related classes ***/

#include "date.hpp"

// day counters
#include "daycounter.hpp"
#include "actual360.hpp"
#include "actual365.hpp"
#include "actualactual.hpp"
#include "thirty360.hpp"
#include "thirty360european.hpp"
#include "thirty360italian.hpp"

// calendars
#include "calendar.hpp"
#include "westerncalendar.hpp"
#include "frankfurt.hpp"
#include "helsinki.hpp"
#include "london.hpp"
#include "milan.hpp"
#include "newyork.hpp"
#include "target.hpp"
#include "wellington.hpp"
#include "zurich.hpp"


/*** currencies ***/

#include "currency.hpp"
#include "dem.hpp"
#include "eur.hpp"
#include "gbp.hpp"
#include "itl.hpp"
#include "usd.hpp"
#include "aud.hpp"
#include "cad.hpp"
#include "chf.hpp"
#include "dkk.hpp"
#include "jpy.hpp"
#include "sek.hpp"


/*** instruments ***/

#include "instrument.hpp"
#include "stock.hpp"


/*** term structures ***/

#include "termstructure.hpp"
#include "piecewiseconstantforwards.hpp"
#include "flatforward.hpp"


/*** volatility surfaces ***/

#include "swaptionvolsurface.hpp"
#include "forwardvolsurface.hpp"


/*** risk functions ***/

#include "riskstatistics.hpp"


/*** math ***/

#include "array.hpp"
#include "matrix.hpp"
#include "symmetricschurdecomposition.hpp"
#include "symmetriceigenvalues.hpp"
#include "multivariateaccumulator.hpp"

// math tools
#include "normaldistribution.hpp"
#include "statistics.hpp"
#include "interpolation.hpp"
#include "linearinterpolation.hpp"
#include "cubicspline.hpp"
#include "lexicographicalview.hpp"

// Monte Carlo tools
#include "boxmuller.hpp"
#include "centrallimitgaussian.hpp"
#include "gaussianarraygenerator.hpp"
#include "gaussianrandomgenerator.hpp"
#include "getcovariance.hpp"
#include "lecuyerrandomgenerator.hpp"
#include "pagodapathpricer.hpp"
#include "standardmultipathgenerator.hpp"
#include "standardpathgenerator.hpp"
#include "uniformrandomgenerator.hpp"

// finite difference tools
#include "backwardeuler.hpp"
#include "bsmoperator.hpp"
#include "boundarycondition.hpp"
#include "cranknicolson.hpp"
#include "dminus.hpp"
#include "dplus.hpp"
#include "dplusdminus.hpp"
#include "dzero.hpp"
#include "finitedifferencemodel.hpp"
#include "forwardeuler.hpp"
#include "identity.hpp"
#include "operator.hpp"
#include "operatortraits.hpp"
#include "stepcondition.hpp"
#include "tridiagonaloperator.hpp"
#include "valueatcenter.hpp"


// 1-D solvers
#include "solver1d.hpp"
#include "bisection.hpp"
#include "brent.hpp"
#include "falseposition.hpp"
#include "newton.hpp"
#include "newtonsafe.hpp"
#include "ridder.hpp"
#include "secant.hpp"


/*** patterns ***/

#include "observable.hpp"


/*** pricers ***/

#include "americancondition.hpp"
#include "americanoption.hpp"
#include "averagepriceasian.hpp"
#include "averagestrikeasian.hpp"
#include "binaryoption.hpp"
#include "barrieroption.hpp"
#include "bsmoption.hpp"
#include "bermudanoption.hpp"
#include "bsmeuropeanoption.hpp"
#include "bsmnumericaloption.hpp"
#include "cliquetoption.hpp"
#include "dividendoption.hpp"
#include "dividendshoutoption.hpp"
#include "dividendeuropeanoption.hpp"
#include "dividendamericanoption.hpp"
#include "everestoption.hpp"
#include "finitedifferenceeuropean.hpp"
#include "geometricasianoption.hpp"
#include "himalaya.hpp"
#include "mceuropeanpricer.hpp"
#include "multiperiodoption.hpp"
#include "options.hpp"
#include "plainbasketoption.hpp"
#include "pagodaoption.hpp"
#include "shoutcondition.hpp"
#include "stepconditionoption.hpp"
#include "shoutoption.hpp"



/*** utilities ***/

#include "handle.hpp"
#include "null.hpp"
#include "dataformatters.hpp"
#include "combiningiterator.hpp"
#include "couplingiterator.hpp"
#include "steppingiterator.hpp"
#include "filteringiterator.hpp"
#include "processingiterator.hpp"
#include "iteratorcategories.hpp"


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
