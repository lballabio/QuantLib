
/*
 Copyright (C) 2003 RiskMap srl

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

#ifndef quantlib_test_solvers_hpp
#define quantlib_test_solvers_hpp

#include <boost/test/unit_test.hpp>

/*! \class QuantLib::Brent <ql/Solvers1D/brent.hpp>

    \test the correctness of the returned values is tested by
          checking them against known good results.
*/

/*! \class QuantLib::Bisection <ql/Solvers1D/bisection.hpp>

    \test the correctness of the returned values is tested by
          checking them against known good results.
*/

/*! \class QuantLib::FalsePosition <ql/Solvers1D/falseposition.hpp>

    \test the correctness of the returned values is tested by
          checking them against known good results.
*/

/*! \class QuantLib::Ridder <ql/Solvers1D/ridder.hpp>

    \test the correctness of the returned values is tested by
          checking them against known good results.
*/

/*! \class QuantLib::Secant <ql/Solvers1D/secant.hpp>

    \test the correctness of the returned values is tested by
          checking them against known good results.
*/

/*! \class QuantLib::Newton <ql/Solvers1D/newton.hpp>

    \test the correctness of the returned values is tested by
          checking them against known good results.
*/

/*! \class QuantLib::NewtonSafe <ql/Solvers1D/newtonsafe.hpp>

    \test the correctness of the returned values is tested by
          checking them against known good results.
*/

class Solver1DTest {
  public:
    static void testResults();
    static boost::unit_test_framework::test_suite* suite();
};


#endif
