
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*
    $Id$
    $Source$
    $Log$
    Revision 1.4  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/


#ifndef quantlib_h
#define quantlib_h

//alphabetical order PLEASE
#include "array.hpp"
#include "calendar.hpp"
#include "currency.hpp"
#include "dataformatters.hpp"
#include "date.hpp"
#include "daycounter.hpp"
#include "depositrate.hpp"
#include "discountfactor.hpp"
#include "expressiontemplates.hpp"
#include "forwardvolsurface.hpp"
#include "handle.hpp"
#include "history.hpp"
#include "instrument.hpp"
#include "null.hpp"
#include "options.hpp"
#include "qldefines.hpp"
#include "qlerrors.hpp"
#include "rate.hpp"
#include "ratehelper.hpp"
#include "riskstatistics.hpp"
#include "solver1d.hpp"
#include "spread.hpp"
#include "swaptionvolsurface.hpp"
#include "termstructure.hpp"

//alphabetical order PLEASE
#include "Calendars/frankfurt.hpp"
#include "Calendars/helsinki.hpp"
#include "Calendars/london.hpp"
#include "Calendars/milan.hpp"
#include "Calendars/newyork.hpp"
#include "Calendars/target.hpp"
#include "Calendars/wellington.hpp"
#include "Calendars/westerncalendar.hpp"
#include "Calendars/zurich.hpp"

//alphabetical order PLEASE
#include "Currencies/aud.hpp"
#include "Currencies/cad.hpp"
#include "Currencies/chf.hpp"
#include "Currencies/dem.hpp"
#include "Currencies/dkk.hpp"
#include "Currencies/eur.hpp"
#include "Currencies/gbp.hpp"
#include "Currencies/itl.hpp"
#include "Currencies/jpy.hpp"
#include "Currencies/sek.hpp"
#include "Currencies/usd.hpp"

//alphabetical order PLEASE
#include "DayCounters/actual360.hpp"
#include "DayCounters/actual365.hpp"
#include "DayCounters/actualactual.hpp"
#include "DayCounters/thirty360.hpp"
#include "DayCounters/thirty360european.hpp"
#include "DayCounters/thirty360italian.hpp"

//alphabetical order PLEASE
#include "FiniteDifferences/backwardeuler.hpp"
#include "FiniteDifferences/boundarycondition.hpp"
#include "FiniteDifferences/bsmoperator.hpp"
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
#include "FiniteDifferences/standardfdmodel.hpp"
#include "FiniteDifferences/standardstepcondition.hpp"
#include "FiniteDifferences/stepcondition.hpp"
#include "FiniteDifferences/tridiagonaloperator.hpp"
#include "FiniteDifferences/valueatcenter.hpp"

//alphabetical order PLEASE
#include "Instruments/stock.hpp"

//alphabetical order PLEASE
#include "Math/cubicspline.hpp"
#include "Math/interpolation.hpp"
#include "Math/lexicographicalview.hpp"
#include "Math/linearinterpolation.hpp"
#include "Math/matrix.hpp"
#include "Math/multivariateaccumulator.hpp"
#include "Math/normaldistribution.hpp"
#include "Math/statistics.hpp"
#include "Math/symmetriceigenvalues.hpp"
#include "Math/symmetricschurdecomposition.hpp"
#include "Math/vartool.hpp"

//alphabetical order PLEASE
#include "MonteCarlo/avgpriceasianpathpricer.hpp"
#include "MonteCarlo/avgstrikeasianpathpricer.hpp"
#include "MonteCarlo/basketpathpricer.hpp"
#include "MonteCarlo/boxmuller.hpp"
#include "MonteCarlo/centrallimitgaussian.hpp"
#include "MonteCarlo/controlvariatedpathpricer.hpp"
#include "MonteCarlo/europeanpathpricer.hpp"
#include "MonteCarlo/everestpathpricer.hpp"
#include "MonteCarlo/gaussianarraygenerator.hpp"
#include "MonteCarlo/gaussianrandomgenerator.hpp"
#include "MonteCarlo/generalmontecarlo.hpp"
#include "MonteCarlo/geometricasianpathpricer.hpp"
#include "MonteCarlo/getcovariance.hpp"
#include "MonteCarlo/himalayapathpricer.hpp"
#include "MonteCarlo/lecuyerrandomgenerator.hpp"
#include "MonteCarlo/mcoptionsample.hpp"
#include "MonteCarlo/mcpricer.hpp"
#include "MonteCarlo/multifactormontecarlooption.hpp"
#include "MonteCarlo/multifactorpricer.hpp"
#include "MonteCarlo/multipath.hpp"
#include "MonteCarlo/multipathgenerator.hpp"
#include "MonteCarlo/multipathpricer.hpp"
#include "MonteCarlo/onefactormontecarlooption.hpp"
#include "MonteCarlo/pagodapathpricer.hpp"
#include "MonteCarlo/path.hpp"
#include "MonteCarlo/pathmontecarlo.hpp"
#include "MonteCarlo/pathpricer.hpp"
#include "MonteCarlo/randomarraygenerator.hpp"
#include "MonteCarlo/standardmultipathgenerator.hpp"
#include "MonteCarlo/standardpathgenerator.hpp"
#include "MonteCarlo/uniformrandomgenerator.hpp"

//alphabetical order PLEASE
#include "Patterns/observable.hpp"

//alphabetical order PLEASE
#include "Pricers/americancondition.hpp"
#include "Pricers/americanoption.hpp"
#include "Pricers/averagepriceasian.hpp"
#include "Pricers/averagestrikeasian.hpp"
#include "Pricers/barrieroption.hpp"
#include "Pricers/bermudanoption.hpp"
#include "Pricers/binaryoption.hpp"
#include "Pricers/bsmeuropeanoption.hpp"
#include "Pricers/bsmnumericaloption.hpp"
#include "Pricers/bsmoption.hpp"
#include "Pricers/cliquetoption.hpp"
#include "Pricers/dividendamericanoption.hpp"
#include "Pricers/dividendeuropeanoption.hpp"
#include "Pricers/dividendoption.hpp"
#include "Pricers/dividendshoutoption.hpp"
#include "Pricers/everestoption.hpp"
#include "Pricers/finitedifferenceeuropean.hpp"
#include "Pricers/geometricasianoption.hpp"
#include "Pricers/himalaya.hpp"
#include "Pricers/mceuropeanpricer.hpp"
#include "Pricers/multiperiodoption.hpp"
#include "Pricers/pagodaoption.hpp"
#include "Pricers/plainbasketoption.hpp"
#include "Pricers/shoutcondition.hpp"
#include "Pricers/shoutoption.hpp"
#include "Pricers/stepconditionoption.hpp"

//alphabetical order PLEASE
#include "Solvers1D/bisection.hpp"
#include "Solvers1D/brent.hpp"
#include "Solvers1D/falseposition.hpp"
#include "Solvers1D/newton.hpp"
#include "Solvers1D/newtonsafe.hpp"
#include "Solvers1D/ridder.hpp"
#include "Solvers1D/secant.hpp"

//alphabetical order PLEASE
#include "TermStructures/flatforward.hpp"
#include "TermStructures/piecewiseconstantforwards.hpp"

//alphabetical order PLEASE
#include "Utilities/combiningiterator.hpp"
#include "Utilities/couplingiterator.hpp"
#include "Utilities/filteringiterator.hpp"
#include "Utilities/iteratorcategories.hpp"
#include "Utilities/processingiterator.hpp"
#include "Utilities/steppingiterator.hpp"



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
