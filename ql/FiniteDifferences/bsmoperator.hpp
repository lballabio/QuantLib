/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2005 StatPro Italia srl

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

/*! \file bsmoperator.hpp
    \brief differential operator for Black-Scholes-Merton equation
*/

#ifndef quantlib_bsm_operator_hpp
#define quantlib_bsm_operator_hpp

#include <ql/FiniteDifferences/tridiagonaloperator.hpp>
#include <ql/Processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Black-Scholes-Merton differential operator
    /*! \ingroup findiff */
    class BSMOperator : public TridiagonalOperator {
      public:
        BSMOperator();
        BSMOperator(Size size, Real dx, Rate r, Rate q, Volatility sigma);
        BSMOperator(const Array& grid,
                    const boost::shared_ptr<BlackScholesProcess>&,
                    Time residualTime);
    };

}


#endif
