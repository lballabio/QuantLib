
/*
 Copyright (C) 2003 RiskMap srl

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

#ifndef quantlib_test_market_elements_hpp
#define quantlib_test_market_elements_hpp

#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>

class MarketElementTest : public CppUnit::TestFixture {
  public:
    void testObservable();
    void testObservableHandle();
    void testDerived();
    void testComposite();
    static CppUnit::Test* suite();
};


#endif
