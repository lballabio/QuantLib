/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_test_gsr_hpp
#define quantlib_test_gsr_hpp

#include <boost/test/unit_test.hpp>

#include "utilities.hpp"
#include <ql/experimental/models/gsrprocess.hpp>
#include <ql/experimental/models/gsr.hpp>
#include <ql/experimental/models/nonstandardswap.hpp>
#include <ql/experimental/models/nonstandardswaption.hpp>
#include <ql/experimental/models/gaussian1dswaptionengine.hpp>
#include <ql/experimental/models/gaussian1djamshidianswaptionengine.hpp>
#include <ql/experimental/models/gaussian1dnonstandardswaptionengine.hpp>

#include <ql/indexes/swap/euriborswap.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/processes/hullwhiteprocess.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/pricingengines/swaption/jamshidianswaptionengine.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/termstructures/volatility/swaption/swaptionconstantvol.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

using namespace QuantLib;

class GsrTest {
  public:
    static void testGsrProcess();
    static void testGsrModel();
    static void testNonstandardSwaption();
    static void testDummy();
    static boost::unit_test_framework::test_suite *suite();
};

#endif
