/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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


/*! \file fdmndimblackscholesop.hpp
*/

#ifndef quantlib_fdm_ndim_black_scholes_op_hpp
#define quantlib_fdm_ndim_black_scholes_op_hpp

#include <ql/math/matrix.hpp>
#include <ql/methods/finitedifferences/operators/ninepointlinearop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>

namespace QuantLib {

    class FdmMesher;
    class FdmBlackScholesOp;
    class LocalVolTermStructure;
    class GeneralizedBlackScholesProcess;

    class FdmndimBlackScholesOp : public FdmLinearOpComposite {
      public:
        FdmndimBlackScholesOp(
            ext::shared_ptr<FdmMesher> mesher,
            std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes,
            Matrix correlation,
            Time maturity);

        Size size() const override;
        void setTime(Time t1, Time t2) override;
        Array apply(const Array& x) const override;
        Array apply_mixed(const Array& x) const override;

        Array apply_direction(Size direction, const Array& x) const override;

        Array solve_splitting(Size direction, const Array& x, Real s) const override;
        Array preconditioner(const Array& r, Real s) const override;

        std::vector<SparseMatrix> toMatrixDecomp() const override;

      private:
        const ext::shared_ptr<FdmMesher> mesher_;
        const std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes_;

        Real currentForwardRate_;
        std::vector<ext::shared_ptr<FdmBlackScholesOp> > ops_;
        std::vector<ext::shared_ptr<NinePointLinearOp> > corrMaps_;
    };
}
#endif
