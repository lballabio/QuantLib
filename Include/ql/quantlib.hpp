
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
    Revision 1.9  2001/05/24 12:52:01  nando
    smoothing #include xx.hpp

    Revision 1.8  2001/05/24 11:15:57  lballabio
    Stripped conventions from Currencies

*/


#ifndef quantlib_h
#define quantlib_h

#include "ql/array.hpp"
#include "ql/calendar.hpp"
#include "ql/currency.hpp"
#include "ql/dataformatters.hpp"
#include "ql/date.hpp"
#include "ql/daycounter.hpp"
#include "ql/depositrate.hpp"
#include "ql/discountfactor.hpp"
#include "ql/expressiontemplates.hpp"
#include "ql/forwardvolsurface.hpp"
#include "ql/handle.hpp"
#include "ql/history.hpp"
#include "ql/index.hpp"
#include "ql/instrument.hpp"
#include "ql/null.hpp"
#include "ql/options.hpp"
#include "ql/qldefines.hpp"
#include "ql/qlerrors.hpp"
#include "ql/rate.hpp"
#include "ql/riskstatistics.hpp"
#include "ql/solver1d.hpp"
#include "ql/spread.hpp"
#include "ql/swaptionvolsurface.hpp"
#include "ql/termstructure.hpp"

#include "ql/Calendars/frankfurt.hpp"
#include "ql/Calendars/helsinki.hpp"
#include "ql/Calendars/london.hpp"
#include "ql/Calendars/milan.hpp"
#include "ql/Calendars/newyork.hpp"
#include "ql/Calendars/target.hpp"
#include "ql/Calendars/wellington.hpp"
#include "ql/Calendars/westerncalendar.hpp"
#include "ql/Calendars/zurich.hpp"

#include "ql/DayCounters/actual360.hpp"
#include "ql/DayCounters/actual365.hpp"
#include "ql/DayCounters/actualactual.hpp"
#include "ql/DayCounters/thirty360.hpp"
#include "ql/DayCounters/thirty360european.hpp"
#include "ql/DayCounters/thirty360italian.hpp"

#include "ql/FiniteDifferences/backwardeuler.hpp"
#include "ql/FiniteDifferences/boundarycondition.hpp"
#include "ql/FiniteDifferences/bsmoperator.hpp"
#include "ql/FiniteDifferences/cranknicolson.hpp"
#include "ql/FiniteDifferences/dminus.hpp"
#include "ql/FiniteDifferences/dplus.hpp"
#include "ql/FiniteDifferences/dplusdminus.hpp"
#include "ql/FiniteDifferences/dzero.hpp"
#include "ql/FiniteDifferences/finitedifferencemodel.hpp"
#include "ql/FiniteDifferences/forwardeuler.hpp"
#include "ql/FiniteDifferences/identity.hpp"
#include "ql/FiniteDifferences/operator.hpp"
#include "ql/FiniteDifferences/operatortraits.hpp"
#include "ql/FiniteDifferences/standardfdmodel.hpp"
#include "ql/FiniteDifferences/standardstepcondition.hpp"
#include "ql/FiniteDifferences/stepcondition.hpp"
#include "ql/FiniteDifferences/tridiagonaloperator.hpp"
#include "ql/FiniteDifferences/valueatcenter.hpp"

#include "ql/Indexes/euribor.hpp"
#include "ql/Indexes/libor.hpp"
#include "ql/Indexes/libormanager.hpp"
#include "ql/Indexes/xibor.hpp"
#include "ql/Indexes/usdlibor.hpp"

#include "ql/Instruments/stock.hpp"

#include "ql/Math/cubicspline.hpp"
#include "ql/Math/interpolation.hpp"
#include "ql/Math/lexicographicalview.hpp"
#include "ql/Math/linearinterpolation.hpp"
#include "ql/Math/matrix.hpp"
#include "ql/Math/multivariateaccumulator.hpp"
#include "ql/Math/normaldistribution.hpp"
#include "ql/Math/statistics.hpp"
#include "ql/Math/symmetriceigenvalues.hpp"
#include "ql/Math/symmetricschurdecomposition.hpp"
#include "ql/Math/vartool.hpp"

#include "ql/MonteCarlo/avgpriceasianpathpricer.hpp"
#include "ql/MonteCarlo/avgstrikeasianpathpricer.hpp"
#include "ql/MonteCarlo/basketpathpricer.hpp"
#include "ql/MonteCarlo/boxmuller.hpp"
#include "ql/MonteCarlo/centrallimitgaussian.hpp"
#include "ql/MonteCarlo/controlvariatedpathpricer.hpp"
#include "ql/MonteCarlo/europeanpathpricer.hpp"
#include "ql/MonteCarlo/everestpathpricer.hpp"
#include "ql/MonteCarlo/gaussianarraygenerator.hpp"
#include "ql/MonteCarlo/gaussianrandomgenerator.hpp"
#include "ql/MonteCarlo/generalmontecarlo.hpp"
#include "ql/MonteCarlo/geometricasianpathpricer.hpp"
#include "ql/MonteCarlo/getcovariance.hpp"
#include "ql/MonteCarlo/himalayapathpricer.hpp"
#include "ql/MonteCarlo/lecuyerrandomgenerator.hpp"
#include "ql/MonteCarlo/mcoptionsample.hpp"
#include "ql/MonteCarlo/mcpricer.hpp"
#include "ql/MonteCarlo/multifactormontecarlooption.hpp"
#include "ql/MonteCarlo/multifactorpricer.hpp"
#include "ql/MonteCarlo/multipath.hpp"
#include "ql/MonteCarlo/multipathgenerator.hpp"
#include "ql/MonteCarlo/multipathpricer.hpp"
#include "ql/MonteCarlo/onefactormontecarlooption.hpp"
#include "ql/MonteCarlo/pagodapathpricer.hpp"
#include "ql/MonteCarlo/path.hpp"
#include "ql/MonteCarlo/pathmontecarlo.hpp"
#include "ql/MonteCarlo/pathpricer.hpp"
#include "ql/MonteCarlo/randomarraygenerator.hpp"
#include "ql/MonteCarlo/standardmultipathgenerator.hpp"
#include "ql/MonteCarlo/standardpathgenerator.hpp"
#include "ql/MonteCarlo/uniformrandomgenerator.hpp"

#include "ql/Patterns/observable.hpp"

#include "ql/Pricers/americancondition.hpp"
#include "ql/Pricers/americanoption.hpp"
#include "ql/Pricers/averagepriceasian.hpp"
#include "ql/Pricers/averagestrikeasian.hpp"
#include "ql/Pricers/barrieroption.hpp"
#include "ql/Pricers/bermudanoption.hpp"
#include "ql/Pricers/binaryoption.hpp"
#include "ql/Pricers/bsmeuropeanoption.hpp"
#include "ql/Pricers/bsmnumericaloption.hpp"
#include "ql/Pricers/bsmoption.hpp"
#include "ql/Pricers/cliquetoption.hpp"
#include "ql/Pricers/dividendamericanoption.hpp"
#include "ql/Pricers/dividendeuropeanoption.hpp"
#include "ql/Pricers/dividendoption.hpp"
#include "ql/Pricers/dividendshoutoption.hpp"
#include "ql/Pricers/everestoption.hpp"
#include "ql/Pricers/finitedifferenceeuropean.hpp"
#include "ql/Pricers/geometricasianoption.hpp"
#include "ql/Pricers/himalaya.hpp"
#include "ql/Pricers/mceuropeanpricer.hpp"
#include "ql/Pricers/multiperiodoption.hpp"
#include "ql/Pricers/pagodaoption.hpp"
#include "ql/Pricers/plainbasketoption.hpp"
#include "ql/Pricers/shoutcondition.hpp"
#include "ql/Pricers/shoutoption.hpp"
#include "ql/Pricers/stepconditionoption.hpp"

#include "ql/Solvers1D/bisection.hpp"
#include "ql/Solvers1D/brent.hpp"
#include "ql/Solvers1D/falseposition.hpp"
#include "ql/Solvers1D/newton.hpp"
#include "ql/Solvers1D/newtonsafe.hpp"
#include "ql/Solvers1D/ridder.hpp"
#include "ql/Solvers1D/secant.hpp"

#include "ql/TermStructures/flatforward.hpp"
#include "ql/TermStructures/piecewiseconstantforwards.hpp"
#include "ql/TermStructures/piecewiseflatforward.hpp"
#include "ql/TermStructures/ratehelpers.hpp"

#include "ql/Utilities/combiningiterator.hpp"
#include "ql/Utilities/couplingiterator.hpp"
#include "ql/Utilities/filteringiterator.hpp"
#include "ql/Utilities/iteratorcategories.hpp"
#include "ql/Utilities/processingiterator.hpp"
#include "ql/Utilities/steppingiterator.hpp"

/*** shortcuts for the full namespaces ***/
namespace QL    = QuantLib;
namespace QLCAL = QuantLib::Calendars;
namespace QLDCO = QuantLib::DayCounters;
namespace QLIDX = QuantLib::Indexes;
namespace QLINS = QuantLib::Instruments;
namespace QLFDM = QuantLib::FiniteDifferences;
namespace QLMTH = QuantLib::Math;
namespace QLMNT = QuantLib::MonteCarlo;
namespace QLPAT = QuantLib::Patterns;
namespace QLPRC = QuantLib::Pricers;
namespace QLS1D = QuantLib::Solvers1D;
namespace QLTST = QuantLib::TermStructures;
namespace QLUTL = QuantLib::Utilities;

#endif
