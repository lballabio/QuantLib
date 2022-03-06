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

#ifndef quantlib_fdm_hull_white_op_hpp
#define quantlib_fdm_hull_white_op_hpp

#include <ql/methods/finitedifferences/operators/triplebandlinearop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>

namespace QuantLib {

    class FdmMesher;
    class HullWhite;

    class FdmHullWhiteOp : public FdmLinearOpComposite {
      public:

        FdmHullWhiteOp(
            const ext::shared_ptr<FdmMesher>& mesher,
            const ext::shared_ptr<HullWhite>& model,
            Size direction);

        Size size() const override;

        //! Time \f$t1 <= t2\f$ is required
        void setTime(Time t1, Time t2) override;

        Disposable<Array> apply(const Array& r) const override;
        Disposable<Array> apply_mixed(const Array& r) const override;
        Disposable<Array> apply_direction(Size direction, const Array& r) const override;
        Disposable<Array> solve_splitting(Size direction, const Array& r, Real s) const override;
        Disposable<Array> preconditioner(const Array& r, Real s) const override;

        Disposable<std::vector<SparseMatrix> > toMatrixDecomp() const override;

      private:
        const Size direction_;
        const Array x_;
        const TripleBandLinearOp dzMap_;
        TripleBandLinearOp mapT_;
        const ext::shared_ptr<HullWhite> model_;
    };
}

#endif


#ifndef id_52e2410df7142d56f00482951b7f7048
#define id_52e2410df7142d56f00482951b7f7048
inline bool test_52e2410df7142d56f00482951b7f7048(const int* i) {
    return i != nullptr;
}
#endif
