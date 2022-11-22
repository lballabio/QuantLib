/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007, 2010, 2012, 2014 Klaus Spanderen
 Copyright (C) 2022 Ignacio Anguita

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

#ifndef quantlib_test_heston_model_hpp
#define quantlib_test_heston_model_hpp

#include <boost/test/unit_test.hpp>
#include "speedlevel.hpp"

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class HestonModelTest {
  public:
    static void testBlackCalibration();
    static void testDAXCalibration();
    static void testAnalyticVsBlack();
    static void testAnalyticVsCached();
    static void testKahlJaeckelCase();
    static void testMcVsCached();
    static void testFdBarrierVsCached();    
    static void testFdVanillaVsCached();    
    static void testDifferentIntegrals();
    static void testMultipleStrikesEngine();
    static void testAnalyticPiecewiseTimeDependent();
    static void testDAXCalibrationOfTimeDependentModel();
    static void testAlanLewisReferencePrices();
    static void testAnalyticPDFHestonEngine();
    static void testExpansionOnAlanLewisReference();
    static void testExpansionOnFordeReference();
    static void testAllIntegrationMethods();
    static void testCosHestonCumulants();
    static void testCosHestonEngine();
    static void testCosHestonEngineTruncation();
    static void testCharacteristicFct();
    static void testAndersenPiterbargPricing();
    static void testAndersenPiterbargControlVariateIntegrand();
    static void testAndersenPiterbargConvergence();
    static void testPiecewiseTimeDependentChFvsHestonChF();
    static void testPiecewiseTimeDependentComparison();
    static void testPiecewiseTimeDependentChFAsymtotic();
    static void testSmallSigmaExpansion();
    static void testSmallSigmaExpansion4ExpFitting();
    static void testExponentialFitting4StrikesAndMaturities();
    static void testHestonEngineIntegration();
    static void testOptimalControlVariateChoice();
    static void testAsymptoticControlVariate();
    static void testLocalVolFromHestonModel();
    

    static boost::unit_test_framework::test_suite* suite(SpeedLevel);
    static boost::unit_test_framework::test_suite* experimental();
};


#endif
