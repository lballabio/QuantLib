
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_h
#define quantlib_h

#include <ql/qldefines.hpp>
#include <ql/core.hpp>

#include <ql/argsandresults.hpp>
#include <ql/capvolstructures.hpp>
#include <ql/dataformatters.hpp>
#include <ql/dataparsers.hpp>
#include <ql/diffusionprocess.hpp>
#include <ql/disposable.hpp>
#include <ql/exercise.hpp>
#include <ql/grid.hpp>
#include <ql/null.hpp>
#include <ql/numericalmethod.hpp>
#include <ql/payoff.hpp>
#include <ql/solver1d.hpp>
#include <ql/swaptionvolstructure.hpp>

#include <ql/Calendars/budapest.hpp>
#include <ql/Calendars/copenhagen.hpp>
#include <ql/Calendars/frankfurt.hpp>
#include <ql/Calendars/helsinki.hpp>
#include <ql/Calendars/johannesburg.hpp>
#include <ql/Calendars/jointcalendar.hpp>
#include <ql/Calendars/london.hpp>
#include <ql/Calendars/milan.hpp>
#include <ql/Calendars/newyork.hpp>
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/Calendars/oslo.hpp>
#include <ql/Calendars/stockholm.hpp>
#include <ql/Calendars/sydney.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/Calendars/tokyo.hpp>
#include <ql/Calendars/toronto.hpp>
#include <ql/Calendars/warsaw.hpp>
#include <ql/Calendars/wellington.hpp>
#include <ql/Calendars/zurich.hpp>

#include <ql/CashFlows/all.hpp>
#include <ql/DayCounters/all.hpp>
#include <ql/FiniteDifferences/all.hpp>

#include <ql/functions/daycounters.hpp>
#include <ql/functions/mathf.hpp>
#include <ql/functions/vols.hpp>

#include <ql/Indexes/all.hpp>
#include <ql/Instruments/all.hpp>

#include <ql/Lattices/binomialtree.hpp>
#include <ql/Lattices/bsmlattice.hpp>
#include <ql/Lattices/lattice.hpp>
#include <ql/Lattices/lattice2d.hpp>
#include <ql/Lattices/tree.hpp>
#include <ql/Lattices/trinomialtree.hpp>

#include <ql/Math/all.hpp>
#include <ql/MonteCarlo/all.hpp>
#include <ql/Optimization/all.hpp>

#include <ql/Patterns/bridge.hpp>
#include <ql/Patterns/composite.hpp>
#include <ql/Patterns/lazyobject.hpp>
#include <ql/Patterns/observable.hpp>
#include <ql/Patterns/visitor.hpp>

#include <ql/Pricers/all.hpp>
#include <ql/PricingEngines/all.hpp>

#include <ql/RandomNumbers/boxmullergaussianrng.hpp>
#include <ql/RandomNumbers/centrallimitgaussianrng.hpp>
#include <ql/RandomNumbers/haltonrsg.hpp>
#include <ql/RandomNumbers/inversecumgaussianrng.hpp>
#include <ql/RandomNumbers/inversecumgaussianrsg.hpp>
#include <ql/RandomNumbers/knuthuniformrng.hpp>
#include <ql/RandomNumbers/lecuyeruniformrng.hpp>
#include <ql/RandomNumbers/mt19937uniformrng.hpp>
#include <ql/RandomNumbers/primitivepolynomials.h>
#include <ql/RandomNumbers/randomarraygenerator.hpp>
#include <ql/RandomNumbers/randomsequencegenerator.hpp>
#include <ql/RandomNumbers/rngtypedefs.hpp>
#include <ql/RandomNumbers/sobolrsg.hpp>

#include <ql/ShortRateModels/all.hpp>
#include <ql/Solvers1D/all.hpp>
#include <ql/TermStructures/all.hpp>

#include <ql/Utilities/combiningiterator.hpp>
#include <ql/Utilities/couplingiterator.hpp>
#include <ql/Utilities/filteringiterator.hpp>
#include <ql/Utilities/iteratorcategories.hpp>
#include <ql/Utilities/processingiterator.hpp>
#include <ql/Utilities/steppingiterator.hpp>

#include <ql/Volatilities/all.hpp>


/*** shortcuts for the full namespaces ***/

//! \deprecated define your own alias when you use QuantLib
namespace QL    = QuantLib;

//! \deprecated inner namespace aliases will be removed in next release
namespace QLCAL = QuantLib::Calendars;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLCFL = QuantLib::CashFlows;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLDCO = QuantLib::DayCounters;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLFDM = QuantLib::FiniteDifferences;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLFUN = QuantLib::Functions;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLIDX = QuantLib::Indexes;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLINS = QuantLib::Instruments;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLMTH = QuantLib::Math;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLMNT = QuantLib::MonteCarlo;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLPAT = QuantLib::Patterns;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLPRC = QuantLib::Pricers;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLPRE = QuantLib::PricingEngines;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLRNG = QuantLib::RandomNumbers;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLSRM = QuantLib::ShortRateModels;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLS1D = QuantLib::Solvers1D;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLTST = QuantLib::TermStructures;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLUTL = QuantLib::Utilities;
//! \deprecated inner namespace aliases will be removed in next release
namespace QLVOL = QuantLib::Volatilities;


#endif
