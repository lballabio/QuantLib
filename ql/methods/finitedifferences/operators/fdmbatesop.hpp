/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdmbatesop.hpp
    \brief Bates linear operator
*/

#ifndef quantlib_fdm_bates_op_hpp
#define quantlib_fdm_bates_op_hpp

#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/methods/finitedifferences/operators/fdmhestonop.hpp>
#include <ql/methods/finitedifferences/utilities/fdmboundaryconditionset.hpp>

namespace QuantLib {

    class LinearInterpolation;
    class BatesProcess;
    
    class FdmBatesOp : public FdmLinearOpComposite {
      public:
        FdmBatesOp(const ext::shared_ptr<FdmMesher>& mesher,
                   const ext::shared_ptr<BatesProcess>& batesProcess,
                   FdmBoundaryConditionSet bcSet,
                   Size integroIntegrationOrder,
                   const ext::shared_ptr<FdmQuantoHelper>& quantoHelper =
                       ext::shared_ptr<FdmQuantoHelper>());

        Size size() const override;
        void setTime(Time t1, Time t2) override;

        Array apply(const Array& r) const override;
        Array apply_mixed(const Array& r) const override;

        Array apply_direction(Size direction, const Array& r) const override;
        Array solve_splitting(Size direction, const Array& r, Real s) const override;
        Array preconditioner(const Array& r, Real s) const override;

        std::vector<SparseMatrix> toMatrixDecomp() const override;

      private:
        class IntegroIntegrand {
          public:
            IntegroIntegrand(const ext::shared_ptr<LinearInterpolation>& i,
                             const FdmBoundaryConditionSet& bcSet,
                             Real x, Real delta, Real nu);
            Real operator()(Real y) const;
            
          private:
            const Real x_, delta_, nu_;
            const FdmBoundaryConditionSet& bcSet_;
            const ext::shared_ptr<LinearInterpolation>& interpl_;
        };
          
        Array integro(const Array& r) const;  
        
        Array x_, weights_;
        
        const Real lambda_, delta_, nu_, m_;
        GaussHermiteIntegration gaussHermiteIntegration_;
        
        const ext::shared_ptr<FdmMesher> mesher_;
        const FdmBoundaryConditionSet bcSet_;
        const ext::shared_ptr<FdmHestonOp> hestonOp_;
    };

    // inline
    inline Size FdmBatesOp::size() const {
        return hestonOp_->size();
    }
    
    inline void FdmBatesOp::setTime(Time t1, Time t2) {
        hestonOp_->setTime(t1, t2);
    }
    
    inline Array FdmBatesOp::apply(const Array& r) const {
        return hestonOp_->apply(r) + integro(r);
    }
    
    inline Array FdmBatesOp::apply_mixed(const Array& r) const {
        return hestonOp_->apply_mixed(r) + integro(r);
    }

    inline Array FdmBatesOp::apply_direction(Size direction,
                                             const Array& r) const {
        return hestonOp_->apply_direction(direction, r);
    }

    inline Array FdmBatesOp::solve_splitting(Size direction,
                                             const Array& r,
                                             Real s) const{
        return hestonOp_->solve_splitting(direction, r, s);
    }
 
    inline Array FdmBatesOp::preconditioner(const Array& r,
                                            Real s) const {
        return hestonOp_->preconditioner(r, s);
    }
    
}

#endif
