
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

// $Id$

#ifndef quantlib_h
#define quantlib_h

// include this first for additional safety
#include <ql/qldefines.hpp>

#include <ql/argsandresults.hpp>
#include <ql/array.hpp>
#include <ql/blackmodel.hpp>
#include <ql/calendar.hpp>
#include <ql/capvolstructures.hpp>
#include <ql/cashflow.hpp>
#include <ql/currency.hpp>
#include <ql/dataformatters.hpp>
#include <ql/dataparsers.hpp>
#include <ql/date.hpp>
#include <ql/daycounter.hpp>
#include <ql/diffusionprocess.hpp>
#include <ql/errors.hpp>
#include <ql/exercise.hpp>
#include <ql/expressiontemplates.hpp>
#include <ql/functions/daycounters.hpp>
#include <ql/functions/mathf.hpp>
#include <ql/functions/vols.hpp>
#include <ql/grid.hpp>
#include <ql/handle.hpp>
#include <ql/history.hpp>
#include <ql/index.hpp>
#include <ql/instrument.hpp>
#include <ql/marketelement.hpp>
#include <ql/null.hpp>
#include <ql/numericalmethod.hpp>
#include <ql/option.hpp>
#include <ql/pricingengine.hpp>
#include <ql/relinkablehandle.hpp>
#include <ql/riskstatistics.hpp>
#include <ql/scheduler.hpp>
#include <ql/solver1d.hpp>
#include <ql/swaptionvolstructure.hpp>
#include <ql/termstructure.hpp>
#include <ql/types.hpp>
#include <ql/voltermstructure.hpp>

#include <ql/Calendars/budapest.hpp>
#include <ql/Calendars/frankfurt.hpp>
#include <ql/Calendars/helsinki.hpp>
#include <ql/Calendars/johannesburg.hpp>
#include <ql/Calendars/jointcalendar.hpp>
#include <ql/Calendars/london.hpp>
#include <ql/Calendars/milan.hpp>
#include <ql/Calendars/newyork.hpp>
#include <ql/Calendars/oslo.hpp>
#include <ql/Calendars/stockholm.hpp>
#include <ql/Calendars/sydney.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/Calendars/tokyo.hpp>
#include <ql/Calendars/toronto.hpp>
#include <ql/Calendars/warsaw.hpp>
#include <ql/Calendars/wellington.hpp>
#include <ql/Calendars/zurich.hpp>

#include <ql/CashFlows/basispointsensitivity.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/coupon.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/CashFlows/inarrearindexedcoupon.hpp>
#include <ql/CashFlows/indexcashflowvectors.hpp>
#include <ql/CashFlows/indexedcoupon.hpp>
#include <ql/CashFlows/parcoupon.hpp>
#include <ql/CashFlows/shortfloatingcoupon.hpp>
#include <ql/CashFlows/shortindexedcoupon.hpp>
#include <ql/CashFlows/simplecashflow.hpp>
#include <ql/CashFlows/upfrontindexedcoupon.hpp>

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
#include <ql/FiniteDifferences/tridiagonaloperator.hpp>
#include <ql/FiniteDifferences/valueatcenter.hpp>

#include <ql/Indexes/audlibor.hpp>
#include <ql/Indexes/cadlibor.hpp>
#include <ql/Indexes/chflibor.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/Indexes/gbplibor.hpp>
#include <ql/Indexes/jpylibor.hpp>
#include <ql/Indexes/usdlibor.hpp>
#include <ql/Indexes/xibor.hpp>
#include <ql/Indexes/xibormanager.hpp>
#include <ql/Indexes/zarlibor.hpp>

#include <ql/Instruments/capfloor.hpp>
#include <ql/Instruments/forwardvanillaoption.hpp>
#include <ql/Instruments/quantoforwardvanillaoption.hpp>
#include <ql/Instruments/quantovanillaoption.hpp>
#include <ql/Instruments/simpleswap.hpp>
#include <ql/Instruments/stock.hpp>
#include <ql/Instruments/swap.hpp>
#include <ql/Instruments/swaption.hpp>
#include <ql/Instruments/vanillaoption.hpp>

#include <ql/Lattices/binomialtree.hpp>
#include <ql/Lattices/bsmlattice.hpp>
#include <ql/Lattices/lattice.hpp>
#include <ql/Lattices/lattice2d.hpp>
#include <ql/Lattices/tree.hpp>
#include <ql/Lattices/trinomialtree.hpp>

#include <ql/Math/bilinearinterpolation.hpp>
#include <ql/Math/bicubicsplineinterpolation.hpp>
#include <ql/Math/chisquaredistribution.hpp>
#include <ql/Math/cubicspline.hpp>
#include <ql/Math/errorfunction.hpp>
#include <ql/Math/gammadistribution.hpp>
#include <ql/Math/interpolation.hpp>
#include <ql/Math/interpolation2D.hpp>
#include <ql/Math/lexicographicalview.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Math/loglinearinterpolation.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Math/multivariateaccumulator.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/Math/primenumbers.hpp>
#include <ql/Math/riskmeasures.hpp>
#include <ql/Math/segmentintegral.hpp>
#include <ql/Math/statistics.hpp>
#include <ql/Math/symmetriceigenvalues.hpp>
#include <ql/Math/symmetricschurdecomposition.hpp>

#include <ql/MonteCarlo/arithmeticapopathpricer.hpp>
#include <ql/MonteCarlo/arithmeticasopathpricer.hpp>
#include <ql/MonteCarlo/basketpathpricer.hpp>
#include <ql/MonteCarlo/cliquetoptionpathpricer.hpp>
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
#include <ql/MonteCarlo/performanceoptionpathpricer.hpp>
#include <ql/MonteCarlo/sample.hpp>

#include <ql/Optimization/armijo.hpp>
#include <ql/Optimization/conjugategradient.hpp>
#include <ql/Optimization/constraint.hpp>
#include <ql/Optimization/costfunction.hpp>
#include <ql/Optimization/criteria.hpp>
#include <ql/Optimization/leastsquare.hpp>
#include <ql/Optimization/linesearch.hpp>
#include <ql/Optimization/method.hpp>
#include <ql/Optimization/problem.hpp>
#include <ql/Optimization/simplex.hpp>
#include <ql/Optimization/steepestdescent.hpp>

#include <ql/Patterns/bridge.hpp>
#include <ql/Patterns/lazyobject.hpp>
#include <ql/Patterns/observable.hpp>
#include <ql/Patterns/visitor.hpp>

#include <ql/Pricers/analyticalcapfloor.hpp>
#include <ql/Pricers/barrieroption.hpp>
#include <ql/Pricers/binaryoption.hpp>
#include <ql/Pricers/blackcapfloor.hpp>
#include <ql/Pricers/blackswaption.hpp>
#include <ql/Pricers/cliquetoption.hpp>
#include <ql/Pricers/continuousgeometricapo.hpp>
#include <ql/Pricers/discretegeometricapo.hpp>
#include <ql/Pricers/discretegeometricaso.hpp>
#include <ql/Pricers/europeanoption.hpp>
#include <ql/Pricers/fdamericanoption.hpp>
#include <ql/Pricers/fdbermudanoption.hpp>
#include <ql/Pricers/fdbsmoption.hpp>
#include <ql/Pricers/fddividendamericanoption.hpp>
#include <ql/Pricers/fddividendeuropeanoption.hpp>
#include <ql/Pricers/fddividendoption.hpp>
#include <ql/Pricers/fddividendshoutoption.hpp>
#include <ql/Pricers/fdeuropean.hpp>
#include <ql/Pricers/fdmultiperiodoption.hpp>
#include <ql/Pricers/fdshoutoption.hpp>
#include <ql/Pricers/fdstepconditionoption.hpp>
#include <ql/Pricers/jamshidianswaption.hpp>
#include <ql/Pricers/mcbasket.hpp>
#include <ql/Pricers/mccliquetoption.hpp>
#include <ql/Pricers/mcdiscretearithmeticapo.hpp>
#include <ql/Pricers/mcdiscretearithmeticaso.hpp>
#include <ql/Pricers/mceuropean.hpp>
#include <ql/Pricers/mceverest.hpp>
#include <ql/Pricers/mchimalaya.hpp>
#include <ql/Pricers/mcmaxbasket.hpp>
#include <ql/Pricers/mcpagoda.hpp>
#include <ql/Pricers/mcperformanceoption.hpp>
#include <ql/Pricers/mcpricer.hpp>
#include <ql/Pricers/performanceoption.hpp>
#include <ql/Pricers/singleassetoption.hpp>
#include <ql/Pricers/treecapfloor.hpp>
#include <ql/Pricers/treeswaption.hpp>

#include <ql/PricingEngines/discretizedvanillaoption.hpp>
#include <ql/PricingEngines/forwardengines.hpp>
#include <ql/PricingEngines/genericengine.hpp>
#include <ql/PricingEngines/latticeshortratemodelengine.hpp>
#include <ql/PricingEngines/mcengine.hpp>
#include <ql/PricingEngines/quantoengines.hpp>
#include <ql/PricingEngines/vanillaengines.hpp>

#include <ql/RandomNumbers/boxmullergaussianrng.hpp>
#include <ql/RandomNumbers/centrallimitgaussianrng.hpp>
#include <ql/RandomNumbers/haltonrsg.hpp>
#include <ql/RandomNumbers/inversecumgaussianrng.hpp>
#include <ql/RandomNumbers/inversecumgaussianrsg.hpp>
#include <ql/RandomNumbers/knuthuniformrng.hpp>
#include <ql/RandomNumbers/lecuyeruniformrng.hpp>
#include <ql/RandomNumbers/randomarraygenerator.hpp>
#include <ql/RandomNumbers/randomsequencegenerator.hpp>
#include <ql/RandomNumbers/rngtypedefs.hpp>

#include <ql/ShortRateModels/CalibrationHelpers/caphelper.hpp>
#include <ql/ShortRateModels/CalibrationHelpers/swaptionhelper.hpp>
#include <ql/ShortRateModels/OneFactorModels/blackkarasinski.hpp>
#include <ql/ShortRateModels/OneFactorModels/coxingersollross.hpp>
#include <ql/ShortRateModels/OneFactorModels/extendedcoxingersollross.hpp>
#include <ql/ShortRateModels/OneFactorModels/hullwhite.hpp>
#include <ql/ShortRateModels/OneFactorModels/vasicek.hpp>
#include <ql/ShortRateModels/TwoFactorModels/g2.hpp>
#include <ql/ShortRateModels/calibrationhelper.hpp>
#include <ql/ShortRateModels/model.hpp>
#include <ql/ShortRateModels/onefactormodel.hpp>
#include <ql/ShortRateModels/parameter.hpp>
#include <ql/ShortRateModels/twofactormodel.hpp>

#include <ql/Solvers1D/bisection.hpp>
#include <ql/Solvers1D/brent.hpp>
#include <ql/Solvers1D/falseposition.hpp>
#include <ql/Solvers1D/newton.hpp>
#include <ql/Solvers1D/newtonsafe.hpp>
#include <ql/Solvers1D/ridder.hpp>
#include <ql/Solvers1D/secant.hpp>

#include <ql/TermStructures/compoundforward.hpp>
#include <ql/TermStructures/discountcurve.hpp>
#include <ql/TermStructures/drifttermstructure.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/TermStructures/forwardspreadedtermstructure.hpp>
#include <ql/TermStructures/impliedtermstructure.hpp>
#include <ql/TermStructures/piecewiseflatforward.hpp>
#include <ql/TermStructures/quantotermstructure.hpp>
#include <ql/TermStructures/ratehelpers.hpp>
#include <ql/TermStructures/zerocurve.hpp>
#include <ql/TermStructures/zerospreadedtermstructure.hpp>

#include <ql/Utilities/combiningiterator.hpp>
#include <ql/Utilities/couplingiterator.hpp>
#include <ql/Utilities/filteringiterator.hpp>
#include <ql/Utilities/iteratorcategories.hpp>
#include <ql/Utilities/processingiterator.hpp>
#include <ql/Utilities/steppingiterator.hpp>

#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Volatilities/blackvariancecurve.hpp>
#include <ql/Volatilities/blackvariancesurface.hpp>
#include <ql/Volatilities/capflatvolvector.hpp>
#include <ql/Volatilities/impliedvoltermstructure.hpp>
#include <ql/Volatilities/localconstantvol.hpp>
#include <ql/Volatilities/localvolcurve.hpp>
#include <ql/Volatilities/localvolsurface.hpp>
#include <ql/Volatilities/swaptionvolmatrix.hpp>


/*** shortcuts for the full namespaces ***/

namespace QL    = QuantLib;
namespace QLCAL = QuantLib::Calendars;
namespace QLCFL = QuantLib::CashFlows;
namespace QLDCO = QuantLib::DayCounters;
namespace QLFDM = QuantLib::FiniteDifferences;
namespace QLIDX = QuantLib::Indexes;
namespace QLINS = QuantLib::Instruments;
namespace QLMTH = QuantLib::Math;
namespace QLMNT = QuantLib::MonteCarlo;
namespace QLPAT = QuantLib::Patterns;
namespace QLPRC = QuantLib::Pricers;
namespace QLPRE = QuantLib::PricingEngines;
namespace QLRNG = QuantLib::RandomNumbers;
namespace QLSRM = QuantLib::ShortRateModels;
namespace QLS1D = QuantLib::Solvers1D;
namespace QLTST = QuantLib::TermStructures;
namespace QLUTL = QuantLib::Utilities;
namespace QLVOL = QuantLib::Volatilities;

#endif
