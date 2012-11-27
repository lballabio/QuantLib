/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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

/*! \file fdmlinearop.hpp
    \brief linear operator to model a multi dimensinal pde system
*/

#ifndef quantlib_fdm_linear_op_hpp
#define quantlib_fdm_linear_op_hpp

#include <ql/math/array.hpp>
#include <ql/math/matrixutilities/sparsematrix.hpp>

namespace QuantLib {

    class FdmLinearOp {
      public:
        typedef Array array_type;
        virtual ~FdmLinearOp() { }
        virtual Disposable<array_type> apply(const array_type& r) const = 0;

#if !defined(QL_NO_UBLAS_SUPPORT)
        virtual Disposable<SparseMatrix> toMatrix() const = 0;
#endif
    };
}

#endif
