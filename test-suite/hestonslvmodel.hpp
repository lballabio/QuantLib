/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014, 2015 Johannes Göttker-Schnetmann
 Copyright (C) 2014, 2015 Klaus Spanderen

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

#ifndef quantlib_test_heston_slv_model_hpp
#define quantlib_test_heston_slv_model_hpp

#include <boost/test/unit_test.hpp>
#include "speedlevel.hpp"

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class HestonSLVModelTest {
public:
    static void testBlackScholesFokkerPlanckFwdEquation();
    static void testSquareRootZeroFlowBC();
    static void testTransformedZeroFlowBC();
    static void testSquareRootEvolveWithStationaryDensity();
    static void testSquareRootLogEvolveWithStationaryDensity();
    static void testSquareRootFokkerPlanckFwdEquation();
    static void testHestonFokkerPlanckFwdEquation();
    static void testHestonFokkerPlanckFwdEquationLogLVLeverage();
    static void testBlackScholesFokkerPlanckFwdEquationLocalVol();
    static void testFDMCalibration();
    static void testLocalVolsvSLVPropDensity();
    static void testBarrierPricingViaHestonLocalVol();
    static void testBarrierPricingMixedModels();
    static void testMonteCarloVsFdmPricing();
    static void testBarrierPricingMixedModelsMonteCarloVsFdmPricing();
    static void testMonteCarloCalibration();
    static void testMoustacheGraph();
    static void testForwardSkewSLV();
    static void testDiffusionAndDriftSlvProcess();

    static boost::unit_test_framework::test_suite* suite(SpeedLevel);

};

#endif
