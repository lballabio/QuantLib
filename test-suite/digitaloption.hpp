
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003 Neil Firth

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

#ifndef quantlib_test_digital_option_hpp
#define quantlib_test_digital_option_hpp

#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>

class DigitalOptionTest : public CppUnit::TestFixture {
  public:
    // European values
    void testCashOrNothingEuropeanValues();
    void testAssetOrNothingEuropeanValues();
    void testGapEuropeanValues();

    // American at-hit values
    void testCashAtHitOrNothingAmericanValues();
    void testAssetAtHitOrNothingAmericanValues();

    // American at-expiry values
    void testCashAtExpiryOrNothingAmericanValues();
    void testAssetAtExpiryOrNothingAmericanValues();

    // European greeks
    // tested in europeanoption.hpp test

    // American at-hit greeks
    void testCashAtHitOrNothingAmericanGreeks();
//    void testAssetAtHitOrNothingAmericanGreeks();


    // American at-expiry greeks
//    void testCashAtExpiryOrNothingAmericanGreeks();
//    void testAssetAtExpiryOrNothingAmericanGreeks();

    void testMCCashAtHit();
    static CppUnit::Test* suite();
};


#endif
