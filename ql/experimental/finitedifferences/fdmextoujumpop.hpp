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

/*! \file fdmextoujumpop.hpp
    \brief Ornstein Uhlenbeck process plus jumps (Kluge Model)
*/

#ifndef quantlib_fdm_ext_ou_jump_op_hpp
#define quantlib_fdm_ext_ou_jump_op_hpp

#include <ql/math/matrixutilities/sparsematrix.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/methods/finitedifferences/operators/triplebandlinearop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmboundaryconditionset.hpp>

namespace QuantLib {
    
    class FdmMesher;
    class YieldTermStructure;
    class LinearInterpolation;
    class ExtOUWithJumpsProcess;
    class FdmExtendedOrnsteinUhlenbackOp;
    
    /*! References:
        Kluge, Timo L., 2008. Pricing Swing Options and other 
        Electricity Derivatives, http://eprints.maths.ox.ac.uk/246/1/kluge.pdf
    */

    class FdmExtOUJumpOp : public FdmLinearOpComposite {
      public:
        FdmExtOUJumpOp(const boost::shared_ptr<FdmMesher>& mesher,
                       const boost::shared_ptr<ExtOUWithJumpsProcess>& process,
                       const boost::shared_ptr<YieldTermStructure>& rTS,
                       const FdmBoundaryConditionSet& bcSet,
                       Size integroIntegrationOrder);

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
        Disposable<Array> integro(const Array& r) const;

        const boost::shared_ptr<FdmMesher> mesher_;
        const boost::shared_ptr<ExtOUWithJumpsProcess> process_;
        const boost::shared_ptr<YieldTermStructure> rTS_;
        const FdmBoundaryConditionSet bcSet_;
        GaussLaguerreIntegration gaussLaguerreIntegration_;

        const Array x_;
        const boost::shared_ptr<FdmExtendedOrnsteinUhlenbackOp> ouOp_;

        const TripleBandLinearOp dyMap_;

#if defined(QL_NO_UBLAS_SUPPORT)
        class IntegroIntegrand {
          public:
            IntegroIntegrand(const boost::shared_ptr<LinearInterpolation>& i,
                             const FdmBoundaryConditionSet& bcSet,
                             Real y, Real eta);
            Real operator()(Real u) const;
            
          private:
            const Real y_, eta_;
            const FdmBoundaryConditionSet& bcSet_;
            const boost::shared_ptr<LinearInterpolation>& interpl_;
        };
            
#else
        SparseMatrix integroPart_;
#endif
    };
}

#endif
