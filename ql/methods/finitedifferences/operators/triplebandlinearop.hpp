/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen
 Copyright (C) 2014 Johannes Göttker-Schnetmann

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

/*! \file triplebandlinearop.hpp
    \brief general triple band linear operator
*/

#ifndef quantlib_triple_band_linear_op_hpp
#define quantlib_triple_band_linear_op_hpp

#include <ql/methods/finitedifferences/operators/fdmlinearop.hpp>
#include <boost/shared_array.hpp>

namespace QuantLib {

    class FdmMesher;
    
    class TripleBandLinearOp : public FdmLinearOp {
      public:
        TripleBandLinearOp(Size direction,
                           const ext::shared_ptr<FdmMesher>& mesher);

        TripleBandLinearOp(const TripleBandLinearOp& m);
        TripleBandLinearOp(const Disposable<TripleBandLinearOp>& m);
        TripleBandLinearOp& operator=(const TripleBandLinearOp& m);
        TripleBandLinearOp& operator=(const Disposable<TripleBandLinearOp>& m);

        Disposable<Array> apply(const Array& r) const;
        Disposable<Array> solve_splitting(const Array& r, Real a,
                                          Real b = 1.0) const;

        Disposable<TripleBandLinearOp> mult(const Array& u) const;
        // interpret u as the diagonal of a diagonal matrix, multiplied on LHS
        Disposable<TripleBandLinearOp> multR(const Array& u) const;
        // interpret u as the diagonal of a diagonal matrix, multiplied on RHS
        Disposable<TripleBandLinearOp> add(const TripleBandLinearOp& m) const;
        Disposable<TripleBandLinearOp> add(const Array& u) const;

        // some very basic linear algebra routines
        void axpyb(const Array& a, const TripleBandLinearOp& x,
                   const TripleBandLinearOp& y, const Array& b);

        void swap(TripleBandLinearOp& m);

#if !defined(QL_NO_UBLAS_SUPPORT)
        Disposable<SparseMatrix> toMatrix() const;
#endif

      protected:
        TripleBandLinearOp() {}

        Size direction_;
        boost::shared_array<Size> i0_, i2_;
        boost::shared_array<Size> reverseIndex_;
        boost::shared_array<Real> lower_, diag_, upper_;

        ext::shared_ptr<FdmMesher> mesher_;
    };
}

#endif
