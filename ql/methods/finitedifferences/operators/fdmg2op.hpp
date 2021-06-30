/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdmhullwhiteop.hpp
    \brief FDM operator for the Hull-White interest rate model
*/

#ifndef quantlib_fdm_g2_op_hpp
#define quantlib_fdm_g2_op_hpp

#include <ql/methods/finitedifferences/operators/ninepointlinearop.hpp>
#include <ql/methods/finitedifferences/operators/triplebandlinearop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>

namespace QuantLib {

    class G2;
    class FdmMesher;

    class FdmG2Op : public FdmLinearOpComposite {
      public:

        FdmG2Op(
            const ext::shared_ptr<FdmMesher>& mesher,
            const ext::shared_ptr<G2>& model,
            Size direction1, Size direction2);

        Size size() const override;
        void setTime(Time t1, Time t2) override;

        Disposable<Array> apply(const Array& r) const override;
        Disposable<Array> apply_mixed(const Array& r) const override;
        Disposable<Array> apply_direction(Size direction, const Array& r) const override;
        Disposable<Array> solve_splitting(Size direction, const Array& r, Real s) const override;
        Disposable<Array> preconditioner(const Array& r, Real s) const override;

        Disposable<std::vector<SparseMatrix> > toMatrixDecomp() const override;

      private:
        const Size direction1_, direction2_;
        const Array x_, y_;
        const TripleBandLinearOp dxMap_, dyMap_;

        NinePointLinearOp corrMap_;
        TripleBandLinearOp mapX_, mapY_;

        const ext::shared_ptr<G2> model_;
    };
}

#endif
