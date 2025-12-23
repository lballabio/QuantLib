/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file spreadoption.hpp
    \brief Spread option on two assets
*/

#ifndef quantlib_spread_option_hpp
#define quantlib_spread_option_hpp

#include <ql/instruments/multiassetoption.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    /*! \deprecated Use BasketOption and KirkEngine instead.
                        Deprecated in version 1.37.
    */
    class [[deprecated("Use BasketOption and KirkEngine instead")]] SpreadOption : public MultiAssetOption {
      public:
        class engine;
        SpreadOption(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                     const ext::shared_ptr<Exercise>& exercise)
        : MultiAssetOption(payoff, exercise) {}
    };

    QL_DEPRECATED_DISABLE_WARNING

    /*! \deprecated Use BasketOption and KirkEngine instead.
                        Deprecated in version 1.37.
    */
    class [[deprecated("Use BasketOption and KirkEngine instead")]] SpreadOption::engine
        : public GenericEngine<SpreadOption::arguments,
                               SpreadOption::results> {};

    QL_DEPRECATED_ENABLE_WARNING

}


#endif
