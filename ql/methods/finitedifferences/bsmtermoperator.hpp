/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

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

/*! \file bsmtermoperator.hpp
    \brief differential operator for Black-Scholes-Merton equation
*/

#ifndef quantlib_bsm_term_operator_hpp
#define quantlib_bsm_term_operator_hpp

#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/transformedgrid.hpp>
#include <ql/methods/finitedifferences/pdebsm.hpp>

namespace QuantLib {

    //! Black-Scholes-Merton differential operator
    /*! \ingroup findiff

        \test coefficients are tested against constant BSM operator
    */
    typedef PdeOperator<PdeBSM> BSMTermOperator;
}


#endif


#ifndef id_4ea7ad35bff45666eb25727d661a010b
#define id_4ea7ad35bff45666eb25727d661a010b
inline bool test_4ea7ad35bff45666eb25727d661a010b(const int* i) {
    return i != nullptr;
}
#endif
