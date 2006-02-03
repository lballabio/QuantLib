/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file problem.hpp
    \brief Abstract optimization class
*/

#ifndef quantlib_optimization_problem_h
#define quantlib_optimization_problem_h

#include <ql/Optimization/method.hpp>

namespace QuantLib {

    //! Constrained optimization problem
    class Problem {
      public:
        //! default constructor
        Problem(CostFunction& f,// Function and it gradient vector
                Constraint& c,  // Constraint
                OptimizationMethod& meth)   // Optimization method
        : costFunction_(f), constraint_(c), method_(meth) {}

        //! call cost function computation and increment evaluation counter
        Real value(const Array& x) const;

        //! call cost values computation and increment evaluation counter
        Disposable<Array> values(const Array& x) const;

        //! call cost function gradient computation and increment
        //  evaluation counter
        void gradient(Array& grad_f, const Array& x) const;

        //! call cost function computation and it gradient
        Real valueAndGradient(Array& grad_f, const Array& x) const;

        //! Constrained optimization method
        OptimizationMethod& method() const { return method_; }

        //! Constraint
        Constraint& constraint() const { return constraint_; }

        //! Cost function
        CostFunction& costFunction() const { return costFunction_; }

        //! Minimization
        void minimize() const { method_.minimize(*this); }

        Array& minimumValue() const { return method_.x (); }

      protected:
        //! Unconstrained cost function
        CostFunction& costFunction_;
        //! Constraint
        Constraint& constraint_;
        //! constrained optimization method
        OptimizationMethod& method_;
    };


    // inline definitions

    inline Real Problem::value(const Array& x) const {
        method_.functionEvaluation()++;
        return costFunction_.value(x);
    }

    inline Disposable<Array> Problem::values(const Array& x) const {
        method_.functionEvaluation()++;
        return costFunction_.values(x);
    }

    inline void Problem::gradient(Array& grad_f, const Array& x) const {
        method_.gradientEvaluation()++;
        costFunction_.gradient(grad_f, x);
    }

    inline Real Problem::valueAndGradient(Array& grad_f,
                                          const Array& x) const {
        method_.functionEvaluation()++;
        method_.gradientEvaluation()++;
        return costFunction_.valueAndGradient(grad_f, x);
    }

}


#endif
