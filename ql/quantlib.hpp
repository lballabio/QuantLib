/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_hpp
#define quantlib_hpp

#include <ql/qldefines.hpp>

/* Use BOOST_MSVC instead of _MSC_VER since some other vendors
   (Metrowerks, for example) also #define _MSC_VER
*/
#ifdef BOOST_MSVC
#  include <ql/auto_link.hpp>
#endif

#include <ql/core.hpp>

#include <ql/time/calendars/all.hpp>
#include <ql/cashflows/all.hpp>
#include <ql/currencies/all.hpp>
#include <ql/time/daycounters/all.hpp>
#include <ql/methods/finitedifferences/all.hpp>
#include <ql/indexes/all.hpp>
#include <ql/instruments/all.hpp>
#include <ql/methods/lattices/all.hpp>
#include <ql/marketmodels/all.hpp>
#include <ql/math/all.hpp>
#include <ql/methods/montecarlo/all.hpp>
#include <ql/math/optimization/all.hpp>
#include <ql/patterns/all.hpp>
#include <ql/pricers/all.hpp>
#include <ql/pricingengines/all.hpp>
#include <ql/processes/all.hpp>
#include <ql/quotes/all.hpp>
#include <ql/math/randomnumbers/all.hpp>
#include <ql/shortratemodels/all.hpp>
#include <ql/math/solvers1d/all.hpp>
#include <ql/termstructures/yieldcurves/all.hpp>
#include <ql/utilities/all.hpp>
#include <ql/termstructures/volatilities/all.hpp>
#include <ql/volatilitymodels/all.hpp>


#endif
