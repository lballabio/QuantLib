
/*
 Copyright (C) 2004 Neil Firth
 Copyright (C) 2003 Ferdinando Ametrano

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

#ifndef quantlib_test_matrices_hpp
#define quantlib_test_matrices_hpp

#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>

class MatricesTest : public CppUnit::TestFixture {
  public:
    void setUp();
    void testEigenvectors();   
    void testSqrt();
    void testSVD();
    static CppUnit::Test* suite();
};


#endif
