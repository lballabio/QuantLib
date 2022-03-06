/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005 StatPro Italia srl

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

/*! \file bsmoperator.hpp
    \brief differential operator for Black-Scholes-Merton equation
*/

#ifndef quantlib_bsm_operator_hpp
#define quantlib_bsm_operator_hpp

#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Black-Scholes-Merton differential operator
    /*! \ingroup findiff */
    class BSMOperator : public TridiagonalOperator {
      public:
        BSMOperator() = default;
        BSMOperator(Size size, Real dx, Rate r, Rate q, Volatility sigma);
        BSMOperator(const Array& grid, Rate r, Rate q, Volatility sigma);
    };

}


#endif


#ifndef id_ae296c0c510acedefd1a248f548ed7e9
#define id_ae296c0c510acedefd1a248f548ed7e9
inline bool test_ae296c0c510acedefd1a248f548ed7e9(int* i) { return i != 0; }
#endif
