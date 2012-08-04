/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008, 2009, 2010 Klaus Spanderen

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

#ifndef quantlib_test_hybrid_heston_hull_white_process_hpp
#define quantlib_test_hybrid_heston_hull_white_process_hpp

#include <boost/test/unit_test.hpp>

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class HybridHestonHullWhiteProcessTest {
  public:
    static void testBsmHullWhiteEngine();
    static void testCompareBsmHWandHestonHW();
    static void testZeroBondPricing();
    static void testMcVanillaPricing();
    static void testMcPureHestonPricing();
    static void testAnalyticHestonHullWhitePricing();
    static void testCallableEquityPricing();
    static void testDiscretizationError();
    static void testFdmHestonHullWhiteEngine();
    static void testBsmHullWhitePricing();
    static void testSpatialDiscretizatinError();
    static void testHestonHullWhiteCalibration();
    static void testH1HWPricingEngine();
    static boost::unit_test_framework::test_suite* suite();
};


#endif
