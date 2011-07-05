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
 <http://quantlib.org/license.shtml>.

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

namespace QuantLib {

    //! Mixed (explicit/implicit) scheme for finite difference methods
    /*! In this implementation, the passed operator must be derived
        from either TimeConstantOperator or TimeDependentOperator.
        Also, it must implement at least the following interface:

        \code
        typedef ... array_type;

        // copy constructor/assignment
        // (these will be provided by the compiler if none is defined)
        Operator(const Operator&);
        Operator& operator=(const Operator&);

        // inspectors
        Size size();

        // modifiers
        void setTime(Time t);

        // operator interface
        array_type applyTo(const array_type&);
        array_type solveFor(const array_type&);
        static Operator identity(Size size);

        // operator algebra
        Operator operator*(Real, const Operator&);
        Operator operator+(const Operator&, const Operator&);
        Operator operator+(const Operator&, const Operator&);
        \endcode

        \warning The differential operator must be linear for
                 this evolver to work.

        \todo
        - derive variable theta schemes
        - introduce multi time-level schemes.

        \ingroup findiff
    */
    template <class Operator>
    class MixedScheme  {
      public:
        // typedefs
        typedef OperatorTraits<Operator> traits;
        typedef typename traits::operator_type operator_type;
        typedef typename traits::array_type array_type;
        typedef typename traits::bc_set bc_set;
        typedef typename traits::condition_type condition_type;
        // constructors
        MixedScheme(const operator_type& L,
                    Real theta,
                    const bc_set& bcs)
        : L_(L), I_(operator_type::identity(L.size())),
          dt_(0.0), theta_(theta) , bcs_(bcs) {}
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
