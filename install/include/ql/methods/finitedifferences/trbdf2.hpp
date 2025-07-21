/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Fabien Le Floc'h

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

/*! \file trbdf2.hpp
    \brief TR-BDF2 scheme for finite difference methods
*/

#ifndef quantlib_trbdf2_hpp
#define quantlib_trbdf2_hpp

#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>

namespace QuantLib {

    //! TR-BDF2 scheme for finite difference methods
    /*! See <http://ssrn.com/abstract=1648878> for details.

        In this implementation, the passed operator must be derived
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

        \ingroup findiff
    */

    // NOTE: There is room for performance improvement especially in
    // the array manipulation

    template <class Operator>
    class TRBDF2  {
      public:
        // typedefs
        typedef OperatorTraits<Operator> traits;
        typedef typename traits::operator_type operator_type;
        typedef typename traits::array_type array_type;
        typedef typename traits::bc_set bc_set;
        typedef typename traits::condition_type condition_type;
        // constructors
        TRBDF2(const operator_type& L,
               const bc_set& bcs)
        : L_(L), I_(operator_type::identity(L.size())),
          dt_(0.0), bcs_(bcs), alpha_(2.0-sqrt(2.0)) {}
        void step(array_type& a,
                  Time t);
        void setStep(Time dt) {
            dt_ = dt;

            implicitPart_ = I_ + 0.5*alpha_*dt_*L_;
            explicitTrapezoidalPart_ = I_ - 0.5*alpha_*dt_*L_;
            explicitBDF2PartFull_ =
                -(1.0-alpha_)*(1.0-alpha_)/(alpha_*(2.0-alpha_))*I_;
            explicitBDF2PartMid_ = 1.0/(alpha_*(2.0-alpha_))*I_;
        }
      private:
        Real alpha_;
        operator_type L_, I_, explicitTrapezoidalPart_,
            explicitBDF2PartFull_,explicitBDF2PartMid_, implicitPart_;
        Time dt_;
        bc_set bcs_;
        array_type aInit_;
    };


    // inline definitions

    template <class Operator>
    inline void TRBDF2<Operator>::step(array_type& a, Time t) {
        Size i;
        Array aInit(a.size());
        for (i=0; i<a.size();i++) {
            aInit[i] = a[i];
        }
        aInit_ = aInit;
        for (i=0; i<bcs_.size(); i++)
            bcs_[i]->setTime(t);
        //trapezoidal explicit part
        if (L_.isTimeDependent()) {
            L_.setTime(t);
            explicitTrapezoidalPart_ = I_ - 0.5*alpha_*dt_*L_;
        }
        for (i=0; i<bcs_.size(); i++)
            bcs_[i]->applyBeforeApplying(explicitTrapezoidalPart_);
        a = explicitTrapezoidalPart_.applyTo(a);
        for (i=0; i<bcs_.size(); i++)
            bcs_[i]->applyAfterApplying(a);

        // trapezoidal implicit part
        if (L_.isTimeDependent()) {
            L_.setTime(t-dt_);
            implicitPart_ = I_ + 0.5*alpha_*dt_*L_;
        }
        for (i=0; i<bcs_.size(); i++)
            bcs_[i]->applyBeforeSolving(implicitPart_,a);
        a = implicitPart_.solveFor(a);
        for (i=0; i<bcs_.size(); i++)
            bcs_[i]->applyAfterSolving(a);


        // BDF2 explicit part
        if (L_.isTimeDependent()) {
            L_.setTime(t);
        }
        for (i=0; i<bcs_.size(); i++) {
            bcs_[i]->applyBeforeApplying(explicitBDF2PartFull_);
        }
        array_type b0 = explicitBDF2PartFull_.applyTo(aInit_);
        for (i=0; i<bcs_.size(); i++)
            bcs_[i]->applyAfterApplying(b0);

        for (i=0; i<bcs_.size(); i++) {
            bcs_[i]->applyBeforeApplying(explicitBDF2PartMid_);
        }
        array_type b1 = explicitBDF2PartMid_.applyTo(a);
        for (i=0; i<bcs_.size(); i++)
            bcs_[i]->applyAfterApplying(b1);
        a = b0+b1;

        // reuse implicit part - works only for alpha=2-sqrt(2)
        for (i=0; i<bcs_.size(); i++)
            bcs_[i]->applyBeforeSolving(implicitPart_,a);
        a = implicitPart_.solveFor(a);
        for (i=0; i<bcs_.size(); i++)
            bcs_[i]->applyAfterSolving(a);

    }

}

#endif
