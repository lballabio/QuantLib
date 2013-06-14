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

/*! \file fdmhestonfwdop.hpp
    \brief Heston Fokker-Planck forward operator
*/

#ifndef quantlib_fdm_heston_fwd_op_hpp
#define quantlib_fdm_heston_fwd_op_hpp

#include <ql/experimental/finitedifferences/fdmsquarerootfwdop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>

namespace QuantLib {

    class YieldTermStructure;
    class FdmMesher;
    class HestonProcess;
    class FirstDerivativeOp;
    class TripleBandLinearOp;
    class NinePointLinearOp;

    class FdmHestonFwdOp : public FdmLinearOpComposite {
      public:
        FdmHestonFwdOp(
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<HestonProcess>& process);

        Size size() const;
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
      private:
        const Real kappa_, theta_, sigma_, rho_, v0_;

        const boost::shared_ptr<YieldTermStructure> rTS_;
        const boost::shared_ptr<YieldTermStructure> qTS_;

        const Array varianceValues_;
        const boost::shared_ptr<FirstDerivativeOp> dxMap_;
        const boost::shared_ptr<ModTripleBandLinearOp> dxxMap_;

        const boost::shared_ptr<TripleBandLinearOp> mapX_;
        const boost::shared_ptr<FdmSquareRootFwdOp> mapY_;

        const boost::shared_ptr<NinePointLinearOp> correlation_;
    };
}

#endif
