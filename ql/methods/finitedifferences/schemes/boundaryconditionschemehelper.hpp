/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Klaus Spanderen

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


/*! \file boundaryconditionschemehelper.hpp
*/

#ifndef quantlib_boundary_condition_scheme_helper_hpp
#define quantlib_boundary_condition_scheme_helper_hpp

#include <ql/methods/finitedifferences/utilities/fdmboundaryconditionset.hpp>

namespace QuantLib {
    class BoundaryConditionSchemeHelper {
      public:
        typedef OperatorTraits<FdmLinearOp>::array_type array_type;
        typedef OperatorTraits<FdmLinearOp>::operator_type operator_type;

        explicit BoundaryConditionSchemeHelper(
            const OperatorTraits<FdmLinearOp>::bc_set& bcSet)
            : bcSet_(bcSet) {  }

        void applyBeforeApplying(operator_type& op) const {
            for (Size i=0; i < bcSet_.size(); ++i)
                bcSet_[i]->applyBeforeApplying(op);
        }
        void applyBeforeSolving(operator_type& op, array_type& a) const {
            for (Size i=0; i < bcSet_.size(); ++i)
                bcSet_[i]->applyBeforeSolving(op, a);
        }
        void applyAfterApplying(array_type& a) const {
            for (Size i=0; i < bcSet_.size(); ++i)
                bcSet_[i]->applyAfterApplying(a);
        }
        void applyAfterSolving(array_type& a) const {
            for (Size i=0; i < bcSet_.size(); ++i)
                bcSet_[i]->applyAfterSolving(a);
        }
        void setTime(Time t) const {
            for (Size i=0; i < bcSet_.size(); ++i)
                bcSet_[i]->setTime(t);
        }

      private:
        BoundaryConditionSchemeHelper() {}
        const OperatorTraits<FdmLinearOp>::bc_set bcSet_;
    };
}

#endif
