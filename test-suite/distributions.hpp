
/*
 Copyright (C) 2003 Ferdinando Ametrano
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

#ifndef quantlib_test_distributions_hpp
#define quantlib_test_distributions_hpp

#include <boost/test/unit_test.hpp>

/*! \class QuantLib::NormalDistribution <ql/Math/normaldistribution.hpp>

    \test the correctness of the returned value is tested by checking
          it against numerical calculations. Cross-checks are also
          performed against the CumulativeNormalDistribution and
          InverseCumulativeNormal classes.
*/

/*! \class QuantLib::BivariateCumulativeNormalDistribution \
    <ql/Math/bivariatenormaldistribution.hpp>

    \test the correctness of the returned value is tested by checking
          it against known good results.
*/

class DistributionTest {
  public:
    static void testNormal();
    static void testBivariate();
    static boost::unit_test_framework::test_suite* suite();
};


#endif
