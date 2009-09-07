/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

#ifndef quantlib_test_inflationvol_hpp
#define quantlib_test_inflationvol_hpp

#include <boost/test/unit_test.hpp>

/* remember to document new and/or updated tests in the Doxygen
 comment block of the corresponding class */

/*!
  \file test/yoyoptionletstripper_test.hpp
  \brief YoY Inflation Cap/Floorlet volatility bootstrap

  Tests bootstrap going from Cap and Floor data, through yoy swap and
  yoy inflation term structure creation, to volatility surface.  The
  only extra data required is the nominal yield curve.

  Since strikes go from negative to positive data is given as prices.
  The optionlet pricer comes in as the yoyCapFloorEngine type.  We
  envisage three:
  * Black (positive strikes only)
  * Unit Displaced Diffusion Black (any strike)
  * Bachelier (any strike)

  \ingroup tests
*/

/* remember to document new and/or updated tests in the Doxygen
 comment block of the corresponding class */


class InflationVolTest {
public:
    static void testYoYPriceSurfaceToATM();
    static void testYoYPriceSurfaceToVol();
    static void testCappedFlooredYoYInflationCoupon();

    static boost::unit_test_framework::test_suite* suite();
};




#endif //quantlib_test_inflationvol_hpp

