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

/*! \file ninepointlinearop.hpp
    \brief nine point linear operator
*/

#ifndef quantlib_nine_point_linear_op_hpp
#define quantlib_nine_point_linear_op_hpp

#include <ql/math/matrixutilities/sparsematrix.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearop.hpp>

#include <boost/shared_array.hpp>

namespace QuantLib {
    class FdmMesher;

    class NinePointLinearOp : public FdmLinearOp {
      public:
        NinePointLinearOp(Size d0, Size d1,
                const boost::shared_ptr<FdmMesher>& mesher);
        NinePointLinearOp(const NinePointLinearOp& m);
        NinePointLinearOp(const Disposable<NinePointLinearOp>& m);
        NinePointLinearOp& operator=(const NinePointLinearOp& m);
        NinePointLinearOp& operator=(const Disposable<NinePointLinearOp>& m);

        Disposable<Array> apply(const Array& r) const;
        Disposable<NinePointLinearOp> mult(const Array& u) const;

        void swap(NinePointLinearOp& m);

#if !defined(QL_NO_UBLAS_SUPPORT)
        Disposable<SparseMatrix> toMatrix() const;
#endif

      protected:
        NinePointLinearOp() {}

        Size d0_, d1_;
        boost::shared_array<Size> i00_, i10_, i20_;
        boost::shared_array<Size> i01_, i21_;
        boost::shared_array<Size> i02_, i12_, i22_;
        boost::shared_array<Real> a00_, a10_, a20_;
        boost::shared_array<Real> a01_, a11_, a21_;
        boost::shared_array<Real> a02_, a12_, a22_;

        boost::shared_ptr<FdmMesher> mesher_;
    };
}

#endif
