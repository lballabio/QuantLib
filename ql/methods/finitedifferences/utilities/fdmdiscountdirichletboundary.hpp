/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2019 Klaus Spanderen

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


/*! \file fdmdiscountdirichletboundary.hpp
    \brief discounted value on Dirichlet boundary conditions
*/

#ifndef quantlib_fdm_discount_dirichlet_boundary_hpp
#define quantlib_fdm_discount_dirichlet_boundary_hpp

#include <ql/methods/finitedifferences/utilities/fdmtimedepdirichletboundary.hpp>

namespace QuantLib {

    class YieldTermStructure;

    class FdmDiscountDirichletBoundary
            : public BoundaryCondition<FdmLinearOp> {
      public:
        typedef FdmLinearOp operator_type;
        typedef FdmLinearOp::array_type array_type;
        typedef BoundaryCondition<FdmLinearOp>::Side Side;

        FdmDiscountDirichletBoundary(
            const ext::shared_ptr<FdmMesher>& mesher,
            const ext::shared_ptr<YieldTermStructure>& rTS,
            Time maturityTime,
            Real valueOnBoundary,
            Size direction, Side side);

        void setTime(Time) override;
        void applyBeforeApplying(operator_type&) const override;
        void applyBeforeSolving(operator_type&, array_type&) const override;
        void applyAfterApplying(array_type&) const override;
        void applyAfterSolving(array_type&) const override;

      private:
        const ext::shared_ptr<FdmTimeDepDirichletBoundary> bc_;
    };
}

#endif
