/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file mixedscheme.hpp
    \brief Mixed (explicit/implicit) scheme for finite difference methods
*/

#ifndef quantlib_mixed_scheme_hpp
#define quantlib_mixed_scheme_hpp

#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>
#include <utility>

namespace QuantLib {

    /*! \deprecated Part of the old FD framework; copy this function
                    in your codebase if needed.
                    Deprecated in version 1.42.
    */
    template <class Operator>
    class [[deprecated("Part of the old FD framework; copy this function in your codebase if needed")]] MixedScheme  {
      public:
        // typedefs
        typedef OperatorTraits<Operator> traits;
        typedef typename traits::operator_type operator_type;
        typedef typename traits::array_type array_type;
        typedef typename traits::bc_set bc_set;
        typedef typename traits::condition_type condition_type;
        // constructors
        MixedScheme(const operator_type& L, Real theta, bc_set bcs)
        : L_(L), I_(operator_type::identity(L.size())), dt_(0.0), theta_(theta),
          bcs_(std::move(bcs)) {}
        void step(array_type& a,
                  Time t);
        void setStep(Time dt) {
            dt_ = dt;
            if (theta_!=1.0) // there is an explicit part
                explicitPart_ = I_-((1.0-theta_) * dt_)*L_;
            if (theta_!=0.0) // there is an implicit part
                implicitPart_ = I_+(theta_ * dt_)*L_;
        }
      protected:
        operator_type L_, I_, explicitPart_, implicitPart_;
        Time dt_;
        Real theta_;
        bc_set bcs_;
    };


    // inline definitions

    template <class Operator>
    inline void MixedScheme<Operator>::step(array_type& a, Time t) {
        Size i;
        for (i=0; i<bcs_.size(); i++)
            bcs_[i]->setTime(t);
        if (theta_!=1.0) { // there is an explicit part
            if (L_.isTimeDependent()) {
                L_.setTime(t);
                explicitPart_ = I_-((1.0-theta_) * dt_)*L_;
            }
            for (i=0; i<bcs_.size(); i++)
                bcs_[i]->applyBeforeApplying(explicitPart_);
            a = explicitPart_.applyTo(a);
            for (i=0; i<bcs_.size(); i++)
                bcs_[i]->applyAfterApplying(a);
        }
        if (theta_!=0.0) { // there is an implicit part
            if (L_.isTimeDependent()) {
                L_.setTime(t-dt_);
                implicitPart_ = I_+(theta_ * dt_)*L_;
            }
            for (i=0; i<bcs_.size(); i++)
                bcs_[i]->applyBeforeSolving(implicitPart_,a);
            implicitPart_.solveFor(a, a);
            for (i=0; i<bcs_.size(); i++)
                bcs_[i]->applyAfterSolving(a);
        }
    }

}


#endif
