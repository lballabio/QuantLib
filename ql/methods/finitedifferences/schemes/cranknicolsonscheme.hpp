/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2019 Klaus Spanderen

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

/*! \file cranknicolsonscheme.hpp
    \brief Crank-Nicolson scheme
*/

#ifndef quantlib_crank_nicolson_scheme_hpp
#define quantlib_crank_nicolson_scheme_hpp

#include <ql/methods/finitedifferences/schemes/impliciteulerscheme.hpp>

namespace QuantLib {

    /*! In one dimension the Crank-Nicolson scheme is equivalent to the
        Douglas scheme and in higher dimensions it is usually inferior to
        operator splitting methods like Craig-Sneyd or Hundsdorfer-Verwer.
    */
    class ExplicitEulerScheme;

    class CrankNicolsonScheme  {
      public:
        // typedefs
        typedef OperatorTraits<FdmLinearOp> traits;
        typedef traits::operator_type operator_type;
        typedef traits::array_type array_type;
        typedef traits::bc_set bc_set;
        typedef traits::condition_type condition_type;

        // constructors
        CrankNicolsonScheme(
            Real theta,
            const std::shared_ptr<FdmLinearOpComposite>& map,
            const bc_set& bcSet = bc_set(),
            Real relTol = 1e-8,
            ImplicitEulerScheme::SolverType solverType
                = ImplicitEulerScheme::BiCGstab);

        void step(array_type& a, Time t);
        void setStep(Time dt);

        Size numberOfIterations() const;
      protected:
        Real dt_;
        const Real theta_;
        const std::shared_ptr<ExplicitEulerScheme> explicit_;
        const std::shared_ptr<ImplicitEulerScheme> implicit_;
    };
}

#endif
