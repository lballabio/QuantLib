/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2004, 2007 Neil Firth
 Copyright (C) 2016 Peter Caspers

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

#ifndef quantlib_test_matrices_hpp
#define quantlib_test_matrices_hpp

#include <boost/test/unit_test.hpp>

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class MatricesTest {
  public:
    static void testEigenvectors();
    static void testSqrt();
    static void testHighamSqrt();
    static void testSVD();
    static void testQRDecomposition();
    static void testQRSolve();
    static void testInverse();
    static void testDeterminant();
    static void testOrthogonalProjection();
    static void testCholeskyDecomposition();
    static void testMoorePenroseInverse();
    static void testIterativeSolvers();
    static void testInitializers();
    static void testSparseMatrixMemory();
    static void testOperators();

    static boost::unit_test_framework::test_suite* suite();
};


#endif
