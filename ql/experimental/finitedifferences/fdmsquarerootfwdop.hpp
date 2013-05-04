/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012, 2013 Klaus Spanderen

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

/*! \file fdmsquarerootfwdop.hpp
    \brief Square root linear operator for the Fokker-Planck forward equation
*/

#ifndef quantlib_fdm_square_root_fwd_op_hpp
#define quantlib_fdm_square_root_fwd_op_hpp

#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>

namespace QuantLib {
    class FdmMesher;
    class SquareRootProcess;
    class TripleBandLinearOp;
    class ModTripleBandLinearOp;

    class FdmSquareRootFwdOp : public FdmLinearOpComposite {
      public:
        FdmSquareRootFwdOp(
            const boost::shared_ptr<FdmMesher>& mesher,
            Real kappa, Real theta, Real sigma,
            Size direction,
            bool transform = false);

        Size size()    const;
        void setTime(Time t1, Time t2);

        Disposable<Array> apply(const Array& r) const;
        Disposable<Array> apply_mixed(const Array& r) const;
        Disposable<Array> apply_direction(Size direction,
                                          const Array& r) const;
        Disposable<Array> solve_splitting(Size direction,
                                          const Array& r, Real s) const;
        Disposable<Array> preconditioner(const Array& r, Real s) const;

#if !defined(QL_NO_UBLAS_SUPPORT)
        Disposable<std::vector<SparseMatrix> > toMatrixDecomp() const;
#endif

        Real f0() const;
        Real f1() const;
        Real v(Size i) const;

      private:
        void setLowerBC(const boost::shared_ptr<FdmMesher>& mesher);
        void setUpperBC(const boost::shared_ptr<FdmMesher>& mesher);
        void setTransformLowerBC(const boost::shared_ptr<FdmMesher>& mesher);
        void setTransformUpperBC(const boost::shared_ptr<FdmMesher>& mesher);

        void getTransformCoeff(Real& alpha, Real& beta,
                               Real& gamma, Size n) const;

        Real h    (Size i) const;
        Real zetam(Size i) const;
        Real zeta (Size i) const;
        Real zetap(Size i) const;
        Real mu   (Size i) const;

        const Size direction_;
        const Real kappa_, theta_, sigma_;
        const Real alpha_;
        const bool transform_;
        boost::shared_ptr<ModTripleBandLinearOp> mapX_;
        Array v_, vq_, vmq_;
    };
}

#endif
