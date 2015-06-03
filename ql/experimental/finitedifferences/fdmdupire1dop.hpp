/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

/*! \file fdmdupire1dop.hpp
    \brief Dupire local volatility pricing operator
        Note that time is reversed in order to make backward solvers work
*/

#ifndef quantlib_fdm_dupire1d_op_hpp
#define quantlib_fdm_dupire1d_op_hpp

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/triplebandlinearop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>

namespace QuantLib {

class FdmDupire1dOp : public FdmLinearOpComposite {
  public:
    FdmDupire1dOp(const boost::shared_ptr<FdmMesher> &mesher,
                  const Array &localVolatility);

    Size size() const;
    void setTime(Time t1, Time t2);

    Disposable<Array> apply(const Array &r) const;
    Disposable<Array> apply_mixed(const Array &r) const;

    Disposable<Array> apply_direction(Size direction, const Array &r) const;
    Disposable<Array> solve_splitting(Size direction, const Array &r,
                                      Real s) const;
    Disposable<Array> preconditioner(const Array &r, Real s) const;

#if !defined(QL_NO_UBLAS_SUPPORT)
    Disposable<std::vector<SparseMatrix> > toMatrixDecomp() const;
#endif

  private:
    const boost::shared_ptr<FdmMesher> mesher_;
    const Array localVolatility_;
    TripleBandLinearOp mapT_;
};
}

#endif
