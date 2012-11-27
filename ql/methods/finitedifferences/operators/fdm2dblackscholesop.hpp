/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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


/*! \file fdm2dblackscholesop.hpp
*/

#ifndef quantlib_fdm_2d_black_scholes_op_hpp
#define quantlib_fdm_2d_black_scholes_op_hpp

#include <ql/methods/finitedifferences/operators/ninepointlinearop.hpp>
#include <ql/methods/finitedifferences/operators/fdmblackscholesop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>

namespace QuantLib {

    class FdmMesher;
    class GeneralizedBlackScholesProcess;

    class Fdm2dBlackScholesOp : public FdmLinearOpComposite {
      public:
        Fdm2dBlackScholesOp(
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& p1,
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& p2,
            Real correlation,
            Time maturity,
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>());
           
        Size size() const;
        void setTime(Time t1, Time t2);    
        Disposable<Array> apply(const Array& x) const;
        Disposable<Array> apply_mixed(const Array& x) const;
    
        Disposable<Array> apply_direction(Size direction,const Array& x) const;
        
        Disposable<Array> solve_splitting(Size direction,
                                          const Array& x, Real s) const;
        Disposable<Array> preconditioner(const Array& r, Real s) const;
    
#if !defined(QL_NO_UBLAS_SUPPORT)
        Disposable<std::vector<SparseMatrix> > toMatrixDecomp() const;
#endif
      private:
        const boost::shared_ptr<FdmMesher> mesher_;
        const boost::shared_ptr<GeneralizedBlackScholesProcess> p1_, p2_;
        const boost::shared_ptr<LocalVolTermStructure> localVol1_, localVol2_;
        const Array x_, y_;
        
        Real currentForwardRate_;
        FdmBlackScholesOp opX_, opY_;
        NinePointLinearOp corrMapT_;
        const NinePointLinearOp corrMapTemplate_;
        const Real illegalLocalVolOverwrite_;
    };
}
#endif
