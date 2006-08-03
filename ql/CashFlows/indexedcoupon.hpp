/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Nicolas Di Césaré

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file indexedcoupon.hpp
    \brief indexed coupon
*/

#ifndef quantlib_indexed_coupon_hpp
#define quantlib_indexed_coupon_hpp

#include <ql/CashFlows/floatingratecoupon.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED
    //! \deprecated use FloatingRateCoupon instead
    typedef FloatingRateCoupon IndexedCoupon;
    #endif

}

#endif
