/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Klaus Spanderen

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

/*! \file nthorderderivativeop.hpp
    \brief n-th order derivative linear operator
*/

#ifndef quantlib_nth_order_derivative_op_hpp
#define quantlib_nth_order_derivative_op_hpp

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearop.hpp>

namespace QuantLib {

    class NthOrderDerivativeOp : public FdmLinearOp {
      public:
        NthOrderDerivativeOp(
            Size direction, Size order, Integer nPoints,
            const ext::shared_ptr<FdmMesher>& mesher);

        array_type apply(const array_type& r) const override;
        SparseMatrix toMatrix() const override;

      private:
        SparseMatrix m_;
    };
}

#endif
