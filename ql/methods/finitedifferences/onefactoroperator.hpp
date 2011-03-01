/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file onefactoroperator.hpp
    \brief general differential operator for one-factor interest rate models
*/

#ifndef quantlib_finite_differences_one_factor_operator_hpp
#define quantlib_finite_differences_one_factor_operator_hpp

#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>
#include <ql/models/shortrate/onefactormodel.hpp>
#include <ql/math/transformedgrid.hpp>
#include <ql/methods/finitedifferences/pdeshortrate.hpp>

namespace QuantLib {

    //! Interest-rate single factor model differential operator
    /*! \ingroup findiff */
    typedef PdeOperator<PdeShortRate> OneFactorOperator;
}


#endif
