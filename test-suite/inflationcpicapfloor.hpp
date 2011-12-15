/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Chris Kenyon
 
 
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


#ifndef quantlib_inflationcpicapfloor_test_hpp
#define quantlib_inflationcpicapfloor_test_hpp


// Boost.Test
#include <boost/test/unit_test.hpp>

//! Tests cpi cap/floor price data
//! N.B. a cpi cap produces ONE flow only - because inflation is cumulative.
class InflationCPICapFloorTest {
public:
    //! tests CPI price surface reproduction
    static void cpicapfloorpricesurface();
    //! tests interpolation pricer
    static void cpicapfloorpricer();
    
    static boost::unit_test_framework::test_suite* suite();
};



#endif
