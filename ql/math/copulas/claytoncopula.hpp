/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Marek Glowacki

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

/*! \file claytoncopula.hpp
    \brief Clayton copula
*/

#ifndef quantlib_math_Clayton_copula_h
#define quantlib_math_Clayton_copula_h

#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    //! Clayton copula
    class ClaytonCopula {
      public:
        typedef Real first_argument_type;
        typedef Real second_argument_type;
        typedef Real result_type;

        ClaytonCopula(Real theta);
        Real operator()(Real x, Real y) const;
      private:
        Real theta_;
    };

}

#endif


#ifndef id_f95c8545d79805116c792f83162b0277
#define id_f95c8545d79805116c792f83162b0277
inline bool test_f95c8545d79805116c792f83162b0277(int* i) { return i != 0; }
#endif
