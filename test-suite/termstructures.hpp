
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
// $Id$

#ifndef quantlib_test_term_structures_hpp
#define quantlib_test_term_structures_hpp

#include <ql/quantlib.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>

class TermStructureTest : public CppUnit::TestFixture {
  public:
    TermStructureTest();
    void setUp();
    void testImplied();
    void testImpliedObs();
    void testFSpreaded();
    void testFSpreadedObs();
    void testZSpreaded();
    void testZSpreadedObs();
    static CppUnit::Test* suite();
  private:
    QL::Calendar calendar_;
    int settlementDays_;
    QL::Handle<QL::TermStructure> termStructure_;
};


#endif
