/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file impliciteuler.hpp
    \brief implicit Euler scheme for finite difference methods
*/

#ifndef quantlib_implicit_euler_hpp
#define quantlib_implicit_euler_hpp

#include <ql/methods/finitedifferences/mixedscheme.hpp>

namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING

    /*! \deprecated Part of the old FD framework; copy this function
                    in your codebase if needed.
                    Deprecated in version 1.42.
    */
    template <class Operator>
    class [[deprecated("Part of the old FD framework; copy this function in your codebase if needed")]] ImplicitEuler : public MixedScheme<Operator> {
      public:
        // typedefs
        typedef OperatorTraits<Operator> traits;
        typedef typename traits::operator_type operator_type;
        typedef typename traits::array_type array_type;
        typedef typename traits::bc_set bc_set;
        typedef typename traits::condition_type condition_type;
        // constructors
        ImplicitEuler(const operator_type& L,
                      const bc_set& bcs)
        : MixedScheme<Operator>(L, 1.0, bcs) {}
    };

    QL_DEPRECATED_ENABLE_WARNING

}


#endif
