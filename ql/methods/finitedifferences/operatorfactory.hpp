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

/*! \file operatorfactory.hpp
    \brief Factory for finite difference operators
*/

#ifndef quantlib_operator_factory_hpp
#define quantlib_operator_factory_hpp

#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/methods/finitedifferences/bsmoperator.hpp>
#include <ql/methods/finitedifferences/bsmtermoperator.hpp>
#include <ql/methods/finitedifferences/onefactoroperator.hpp>

namespace QuantLib {

    //! Factory for finite-difference operators
    /*! \deprecated Instantiate operators directly.
                    Deprecated in version 1.14.
    */
    class QL_DEPRECATED OperatorFactory {
      public:
        static TridiagonalOperator getOperator(
             const ext::shared_ptr<GeneralizedBlackScholesProcess> &process,
             const Array &grid,
             Time residualTime,
             bool timeDependent) {
            if (timeDependent)
                return BSMTermOperator(grid, process, residualTime);
            else
#if defined(QL_PATCH_MSVC)
#pragma warning(push)
#pragma warning(disable:4996)
#endif
                return BSMOperator(grid, process, residualTime);
#if defined(QL_PATCH_MSVC)
#pragma warning(pop)
#endif
        };
        static TridiagonalOperator getOperator(
          const ext::shared_ptr<OneFactorModel::ShortRateDynamics> &process,
          const Array &grid) {
            return OneFactorOperator(grid, process);
        }
    };

}


#endif
