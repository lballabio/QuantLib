
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
    Revision 1.2  2001/04/04 12:13:22  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

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
#include "DayCounters/actual360.hpp"
#include "DayCounters/actual365.hpp"
#include "DayCounters/actualactual.hpp"
#include "DayCounters/thirty360.hpp"
#include "DayCounters/thirty360european.hpp"
#include "DayCounters/thirty360italian.hpp"

// calendars
#include "calendar.hpp"
#include "Calendars/westerncalendar.hpp"
#include "Calendars/frankfurt.hpp"
#include "Calendars/helsinki.hpp"
#include "Calendars/london.hpp"
#include "Calendars/milan.hpp"
#include "Calendars/newyork.hpp"
#include "Calendars/target.hpp"
#include "Calendars/wellington.hpp"
#include "Calendars/zurich.hpp"


/*** currencies ***/

#include "currency.hpp"
#include "Currencies/aud.hpp"
#include "Currencies/cad.hpp"
#include "Currencies/chf.hpp"
#include "Currencies/dkk.hpp"
#include "Currencies/dem.hpp"
#include "Currencies/eur.hpp"
#include "Currencies/gbp.hpp"
#include "Currencies/itl.hpp"
#include "Currencies/jpy.hpp"
#include "Currencies/sek.hpp"
#include "Currencies/usd.hpp"


/*** instruments ***/

#include "instrument.hpp"
#include "Instruments/stock.hpp"


/*** term structures ***/

#include "termstructure.hpp"
#include "TermStructures/piecewiseconstantforwards.hpp"
#include "TermStructures/flatforward.hpp"


/*** volatility surfaces ***/

#include "swaptionvolsurface.hpp"
#include "forwardvolsurface.hpp"


/*** risk functions ***/

#include "riskstatistics.hpp"


/*** math ***/

#include "array.hpp"
#include "Math/matrix.hpp"
#include "Math/symmetricschurdecomposition.hpp"
#include "Math/symmetriceigenvalues.hpp"
#include "Math/multivariateaccumulator.hpp"

// math tools
#include "Math/cubicspline.hpp"
#include "Math/interpolation.hpp"
#include "Math/lexicographicalview.hpp"
#include "Math/linearinterpolation.hpp"
#include "Math/normaldistribution.hpp"
#include "Math/statistics.hpp"

// Monte Carlo tools
#include "MonteCarlo/boxmuller.hpp"
#include "MonteCarlo/centrallimitgaussian.hpp"
#include "MonteCarlo/gaussianarraygenerator.hpp"
#include "MonteCarlo/gaussianrandomgenerator.hpp"
#include "MonteCarlo/getcovariance.hpp"
#include "MonteCarlo/lecuyerrandomgenerator.hpp"
#include "MonteCarlo/pagodapathpricer.hpp"
#include "MonteCarlo/standardmultipathgenerator.hpp"
#include "MonteCarlo/standardpathgenerator.hpp"
#include "MonteCarlo/uniformrandomgenerator.hpp"

// finite difference tools
#include "FiniteDifferences/backwardeuler.hpp"
#include "FiniteDifferences/bsmoperator.hpp"
#include "FiniteDifferences/boundarycondition.hpp"
#include "FiniteDifferences/cranknicolson.hpp"
#include "FiniteDifferences/dminus.hpp"
#include "FiniteDifferences/dplus.hpp"
#include "FiniteDifferences/dplusdminus.hpp"
#include "FiniteDifferences/dzero.hpp"
#include "FiniteDifferences/finitedifferencemodel.hpp"
#include "FiniteDifferences/forwardeuler.hpp"
#include "FiniteDifferences/identity.hpp"
#include "FiniteDifferences/operator.hpp"
#include "FiniteDifferences/operatortraits.hpp"
#include "FiniteDifferences/stepcondition.hpp"
#include "FiniteDifferences/tridiagonaloperator.hpp"
#include "FiniteDifferences/valueatcenter.hpp"


// 1-D solvers
#include "solver1d.hpp"
#include "Solvers1D/bisection.hpp"
#include "Solvers1D/brent.hpp"
#include "Solvers1D/falseposition.hpp"
#include "Solvers1D/newton.hpp"
#include "Solvers1D/newtonsafe.hpp"
#include "Solvers1D/ridder.hpp"
#include "Solvers1D/secant.hpp"


/*** patterns ***/

#include "Patterns/observable.hpp"


/*** pricers ***/

#include "Pricers/americancondition.hpp"
#include "Pricers/americanoption.hpp"
#include "Pricers/averagepriceasian.hpp"
#include "Pricers/averagestrikeasian.hpp"
#include "Pricers/binaryoption.hpp"
#include "Pricers/barrieroption.hpp"
#include "Pricers/bsmoption.hpp"
#include "Pricers/bermudanoption.hpp"
#include "Pricers/bsmeuropeanoption.hpp"
#include "Pricers/bsmnumericaloption.hpp"
#include "Pricers/cliquetoption.hpp"
#include "Pricers/dividendoption.hpp"
#include "Pricers/dividendshoutoption.hpp"
#include "Pricers/dividendeuropeanoption.hpp"
#include "Pricers/dividendamericanoption.hpp"
#include "Pricers/everestoption.hpp"
#include "Pricers/finitedifferenceeuropean.hpp"
#include "Pricers/geometricasianoption.hpp"
#include "Pricers/himalaya.hpp"
#include "Pricers/mceuropeanpricer.hpp"
#include "Pricers/multiperiodoption.hpp"
#include "options.hpp"
#include "Pricers/plainbasketoption.hpp"
#include "Pricers/pagodaoption.hpp"
#include "Pricers/shoutcondition.hpp"
#include "Pricers/stepconditionoption.hpp"
#include "Pricers/shoutoption.hpp"



/*** utilities ***/

#include "handle.hpp"
#include "null.hpp"
#include "dataformatters.hpp"
#include "Utilities/combiningiterator.hpp"
#include "Utilities/couplingiterator.hpp"
#include "Utilities/steppingiterator.hpp"
#include "Utilities/filteringiterator.hpp"
#include "Utilities/processingiterator.hpp"
#include "Utilities/iteratorcategories.hpp"


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
