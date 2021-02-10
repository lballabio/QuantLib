/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Andreas Gaida
 Copyright (C) 2009 Ralph Schreyer
 Copyright (C) 2009, 2017 Klaus Spanderen

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

/*! \file impliciteulerscheme.hpp
    \brief Implicit-Euler scheme
*/

#ifndef quantlib_implicit_euler_scheme_hpp
#define quantlib_implicit_euler_scheme_hpp

#include <ql/methods/finitedifferences/operatortraits.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>
#include <ql/methods/finitedifferences/schemes/boundaryconditionschemehelper.hpp>

namespace QuantLib {

    class ImplicitEulerScheme {
      public:
        enum SolverType { BiCGstab, GMRES };

        // typedefs
        typedef OperatorTraits<FdmLinearOp> traits;
        typedef traits::operator_type operator_type;
        typedef traits::array_type array_type;
        typedef traits::bc_set bc_set;
        typedef traits::condition_type condition_type;

        // constructors
        explicit ImplicitEulerScheme(ext::shared_ptr<FdmLinearOpComposite> map,
                                     const bc_set& bcSet = bc_set(),
                                     Real relTol = 1e-8,
                                     SolverType solverType = BiCGstab);

        void step(array_type& a, Time t);
        void setStep(Time dt);

        Size numberOfIterations() const;
      protected:
        friend class CrankNicolsonScheme;
        void step(array_type& a, Time t, Real theta);

        Disposable<Array> apply(const Array& r, Real theta) const;
          
        Time dt_;
        ext::shared_ptr<Size> iterations_;

        const Real relTol_;
        const ext::shared_ptr<FdmLinearOpComposite> map_;
        const BoundaryConditionSchemeHelper bcSet_;
        const SolverType solverType_;
    };
}

#endif
