/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2026 Kyrylo Protsenko

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

/*! \file method.hpp
    \brief Abstract optimization method class
*/

#ifndef quantlib_optimization_method_h
#define quantlib_optimization_method_h

#include <ql/math/optimization/endcriteria.hpp>

namespace QuantLib {

    class Problem;
    
    //! Abstract class for constrained optimization method
    class OptimizationMethod {
      public:
        virtual ~OptimizationMethod() = default;

        /*! Whether this method consumes CostFunction::jacobian().

            Optimizers that return false may still use derivatives obtained
            through another interface, but they cannot be used when a caller
            explicitly requires the cost-function Jacobian to be consumed.
        */
        virtual bool usesCostFunctionJacobian() const { return false; }

        //! minimize the optimization problem P
        virtual EndCriteria::Type minimize(Problem& P,
                                           const EndCriteria& endCriteria) = 0;
    };

}

#endif
