
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000-2004 StatPro Italia srl

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

#ifndef quantlib_hpp
#define quantlib_hpp

#include <ql/qldefines.hpp>

#include <ql/core.hpp>

#include <ql/Calendars/all.hpp>
#include <ql/CashFlows/all.hpp>
#include <ql/DayCounters/all.hpp>
#include <ql/FiniteDifferences/all.hpp>
#include <ql/Indexes/all.hpp>
#include <ql/Instruments/all.hpp>
#include <ql/Lattices/all.hpp>
#include <ql/Math/all.hpp>
#include <ql/MonteCarlo/all.hpp>
#include <ql/Optimization/all.hpp>
#include <ql/Patterns/all.hpp>
#include <ql/Pricers/all.hpp>
#include <ql/PricingEngines/all.hpp>
#include <ql/RandomNumbers/all.hpp>
#include <ql/ShortRateModels/all.hpp>
#include <ql/Solvers1D/all.hpp>
#include <ql/TermStructures/all.hpp>
#include <ql/Utilities/all.hpp>
#include <ql/Volatilities/all.hpp>
#include <ql/functions/all.hpp>


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
