/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

// $Id$

#ifndef quantlib_h
#define quantlib_h

// include this first for additional safety
#include <ql/qldefines.hpp>

#include <ql/argsandresults.hpp>
#include <ql/array.hpp>
#include <ql/calendar.hpp>
#include <ql/cashflow.hpp>
#include <ql/constraint.hpp>
#include <ql/currency.hpp>
#include <ql/dataformatters.hpp>
#include <ql/date.hpp>
#include <ql/daycounter.hpp>
#include <ql/errors.hpp>
#include <ql/expressiontemplates.hpp>
#include <ql/forwardvolsurface.hpp>
#include <ql/handle.hpp>
#include <ql/history.hpp>
#include <ql/index.hpp>
#include <ql/instrument.hpp>
#include <ql/marketelement.hpp>
#include <ql/minimizer.hpp>
#include <ql/null.hpp>
#include <ql/option.hpp>
#include <ql/relinkablehandle.hpp>
#include <ql/riskstatistics.hpp>
#include <ql/scheduler.hpp>
#include <ql/solver1d.hpp>
#include <ql/stochasticprocess.hpp>
#include <ql/swaptionvolsurface.hpp>
#include <ql/termstructure.hpp>
#include <ql/types.hpp>

#include <ql/Calendars/frankfurt.hpp>
#include <ql/Calendars/helsinki.hpp>
#include <ql/Calendars/london.hpp>
#include <ql/Calendars/milan.hpp>
#include <ql/Calendars/newyork.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/Calendars/wellington.hpp>
#include <ql/Calendars/zurich.hpp>

#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/coupon.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/CashFlows/simplecashflow.hpp>

#include <ql/DayCounters/actual360.hpp>
#include <ql/DayCounters/actual365.hpp>
#include <ql/DayCounters/actualactual.hpp>
#include <ql/DayCounters/thirty360.hpp>

#include <ql/FiniteDifferences/americancondition.hpp>
#include <ql/FiniteDifferences/boundarycondition.hpp>
#include <ql/FiniteDifferences/bsmoperator.hpp>
#include <ql/FiniteDifferences/cranknicolson.hpp>
#include <ql/FiniteDifferences/dminus.hpp>
#include <ql/FiniteDifferences/dplus.hpp>
#include <ql/FiniteDifferences/dplusdminus.hpp>
#include <ql/FiniteDifferences/dzero.hpp>
#include <ql/FiniteDifferences/expliciteuler.hpp>
#include <ql/FiniteDifferences/fdtypedefs.hpp>
#include <ql/FiniteDifferences/finitedifferencemodel.hpp>
#include <ql/FiniteDifferences/impliciteuler.hpp>
#include <ql/FiniteDifferences/onefactoroperator.hpp>
#include <ql/FiniteDifferences/shoutcondition.hpp>
#include <ql/FiniteDifferences/stepcondition.hpp>
#include <ql/FiniteDifferences/swaptioncondition.hpp>
#include <ql/FiniteDifferences/tridiagonaloperator.hpp>
#include <ql/FiniteDifferences/valueatcenter.hpp>

#include <ql/Indexes/euribor.hpp>
#include <ql/Indexes/gbplibor.hpp>
#include <ql/Indexes/usdlibor.hpp>
#include <ql/Indexes/xibor.hpp>
#include <ql/Indexes/xibormanager.hpp>

#include <ql/Instruments/capfloor.hpp>
#include <ql/Instruments/europeanswaption.hpp>
#include <ql/Instruments/plainoption.hpp>
#include <ql/Instruments/simpleswap.hpp>
#include <ql/Instruments/stock.hpp>
#include <ql/Instruments/swap.hpp>

#include <ql/InterestRateModelling/grid.hpp>
#include <ql/InterestRateModelling/model.hpp>
#include <ql/InterestRateModelling/onefactormodel.hpp>
#include <ql/InterestRateModelling/swapfuturevalue.hpp>

#include <ql/InterestRateModelling/CalibrationHelpers/cap.hpp>
#include <ql/InterestRateModelling/CalibrationHelpers/swaption.hpp>

#include <ql/InterestRateModelling/OneFactorModels/blackdermanandtoy.hpp>
#include <ql/InterestRateModelling/OneFactorModels/blackkarasinski.hpp>
#include <ql/InterestRateModelling/OneFactorModels/hoandlee.hpp>
#include <ql/InterestRateModelling/OneFactorModels/hullandwhite.hpp>
#include <ql/InterestRateModelling/OneFactorModels/node.hpp>
#include <ql/InterestRateModelling/OneFactorModels/tree.hpp>

#include <ql/Math/cubicspline.hpp>
#include <ql/Math/interpolation.hpp>
#include <ql/Math/lexicographicalview.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Math/multivariateaccumulator.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/Math/riskmeasures.hpp>
#include <ql/Math/segmentintegral.hpp>
#include <ql/Math/statistics.hpp>
#include <ql/Math/symmetriceigenvalues.hpp>
#include <ql/Math/symmetricschurdecomposition.hpp>

#include <ql/MonteCarlo/arithmeticapopathpricer.hpp>
#include <ql/MonteCarlo/arithmeticasopathpricer.hpp>
#include <ql/MonteCarlo/basketpathpricer.hpp>
#include <ql/MonteCarlo/europeanpathpricer.hpp>
#include <ql/MonteCarlo/everestpathpricer.hpp>
#include <ql/MonteCarlo/geometricapopathpricer.hpp>
#include <ql/MonteCarlo/geometricasopathpricer.hpp>
#include <ql/MonteCarlo/getcovariance.hpp>
#include <ql/MonteCarlo/himalayapathpricer.hpp>
#include <ql/MonteCarlo/maxbasketpathpricer.hpp>
#include <ql/MonteCarlo/mctypedefs.hpp>
#include <ql/MonteCarlo/montecarlomodel.hpp>
#include <ql/MonteCarlo/multipath.hpp>
#include <ql/MonteCarlo/multipathgenerator.hpp>
#include <ql/MonteCarlo/pagodapathpricer.hpp>
#include <ql/MonteCarlo/path.hpp>
#include <ql/MonteCarlo/pathgenerator.hpp>
#include <ql/MonteCarlo/pathpricer.hpp>
#include <ql/MonteCarlo/sample.hpp>

#include <ql/Optimization/armijo.hpp>
#include <ql/Optimization/conjugategradient.hpp>
#include <ql/Optimization/costfunction.hpp>
#include <ql/Optimization/criteria.hpp>
#include <ql/Optimization/leastsquare.hpp>
#include <ql/Optimization/linesearch.hpp>
#include <ql/Optimization/optimizer.hpp>
#include <ql/Optimization/steepestdescent.hpp>

#include <ql/Patterns/factory.hpp>
#include <ql/Patterns/observable.hpp>

#include <ql/Pricers/barrieroption.hpp>
#include <ql/Pricers/binaryoption.hpp>
#include <ql/Pricers/cliquetoption.hpp>
#include <ql/Pricers/continuousgeometricapo.hpp>
#include <ql/Pricers/couponbondoption.hpp>
#include <ql/Pricers/discretegeometricapo.hpp>
#include <ql/Pricers/discretegeometricaso.hpp>
#include <ql/Pricers/europeanengine.hpp>
#include <ql/Pricers/europeanoption.hpp>
#include <ql/Pricers/fdamericanoption.hpp>
#include <ql/Pricers/fdbermudanoption.hpp>
#include <ql/Pricers/fdbsmoption.hpp>
#include <ql/Pricers/fddividendamericanoption.hpp>
#include <ql/Pricers/fddividendeuropeanoption.hpp>
#include <ql/Pricers/fddividendoption.hpp>
#include <ql/Pricers/fddividendshoutoption.hpp>
#include <ql/Pricers/fdeuropean.hpp>
#include <ql/Pricers/fdeuropeanswaption.hpp>
#include <ql/Pricers/fdmultiperiodoption.hpp>
#include <ql/Pricers/fdshoutoption.hpp>
#include <ql/Pricers/fdstepconditionoption.hpp>
#include <ql/Pricers/mcbasket.hpp>
#include <ql/Pricers/mcdiscretearithmeticapo.hpp>
#include <ql/Pricers/mcdiscretearithmeticaso.hpp>
#include <ql/Pricers/mceuropean.hpp>
#include <ql/Pricers/mceverest.hpp>
#include <ql/Pricers/mchimalaya.hpp>
#include <ql/Pricers/mcmaxbasket.hpp>
#include <ql/Pricers/mcpagoda.hpp>
#include <ql/Pricers/mcpricer.hpp>
#include <ql/Pricers/singleassetoption.hpp>

#include <ql/RandomNumbers/boxmullergaussianrng.hpp>
#include <ql/RandomNumbers/centrallimitgaussianrng.hpp>
#include <ql/RandomNumbers/inversecumulativegaussianrng.hpp>
#include <ql/RandomNumbers/knuthuniformrng.hpp>
#include <ql/RandomNumbers/lecuyeruniformrng.hpp>
#include <ql/RandomNumbers/randomarraygenerator.hpp>
#include <ql/RandomNumbers/rngtypedefs.hpp>
//#include <ql/RandomNumbers/brodasoboluniformlds.hpp>

#include <ql/Solvers1D/bisection.hpp>
#include <ql/Solvers1D/brent.hpp>
#include <ql/Solvers1D/falseposition.hpp>
#include <ql/Solvers1D/newton.hpp>
#include <ql/Solvers1D/newtonsafe.hpp>
#include <ql/Solvers1D/ridder.hpp>
#include <ql/Solvers1D/secant.hpp>

#include <ql/TermStructures/flatforward.hpp>
#include <ql/TermStructures/piecewiseflatforward.hpp>
#include <ql/TermStructures/ratehelpers.hpp>

#include <ql/Utilities/combiningiterator.hpp>
#include <ql/Utilities/couplingiterator.hpp>
#include <ql/Utilities/filteringiterator.hpp>
#include <ql/Utilities/iteratorcategories.hpp>
#include <ql/Utilities/processingiterator.hpp>
#include <ql/Utilities/steppingiterator.hpp>


/*** shortcuts for the full namespaces ***/
namespace QL    = QuantLib;
namespace QLCAL = QuantLib::Calendars;
namespace QLCFL = QuantLib::CashFlows;
namespace QLDCO = QuantLib::DayCounters;
namespace QLIDX = QuantLib::Indexes;
namespace QLINS = QuantLib::Instruments;
namespace QLFDM = QuantLib::FiniteDifferences;
namespace QLMTH = QuantLib::Math;
namespace QLMNT = QuantLib::MonteCarlo;
namespace QLPAT = QuantLib::Patterns;
namespace QLPRC = QuantLib::Pricers;
namespace QLRNG = QuantLib::RandomNumbers;
namespace QLS1D = QuantLib::Solvers1D;
namespace QLTST = QuantLib::TermStructures;
namespace QLUTL = QuantLib::Utilities;


#endif
