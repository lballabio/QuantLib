/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Marcin Rybacki

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file rateaveraging.hpp
    \brief rate-averaging method
*/

#ifndef quantlib_rate_averaging_hpp
#define quantlib_rate_averaging_hpp

#include <ql/qldefines.hpp>

namespace QuantLib {

    //! rate averaging method
    /*! It allows to configure how interest is accrued in multi-fixing
        coupons or futures.
    */
    struct RateAveraging {
        enum Type {
            Simple,  /*!< Under the simple convention the amount of
                          interest is calculated by applying the
                          sub-rate to the principal, and the payment
                          due at the end of the period is the sum of
                          those amounts. */
            Compound /*!< Under the compound convention, the
                          additional amount of interest owed each
                          period is calculated by applying the rate
                          both to the principal and the accumulated
                          unpaid interest. */
        };
    };

}

#endif
