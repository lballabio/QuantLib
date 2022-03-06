/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Andreas Gaida
 Copyright (C) 2009 Ralph Schreyer
 Copyright (C) 2009 Klaus Spanderen

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

/*! \file expliciteulerscheme.hpp
    \brief explicit-Euler scheme
*/

#ifndef quantlib_explicit_euler_scheme_hpp
#define quantlib_explicit_euler_scheme_hpp

#include <ql/methods/finitedifferences/operatortraits.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>
#include <ql/methods/finitedifferences/schemes/boundaryconditionschemehelper.hpp>

namespace QuantLib {

    class ExplicitEulerScheme  {
      public:
        // typedefs
        typedef OperatorTraits<FdmLinearOp> traits;
        typedef traits::operator_type operator_type;
        typedef traits::array_type array_type;
        typedef traits::bc_set bc_set;
        typedef traits::condition_type condition_type;

        // constructors
        explicit ExplicitEulerScheme(ext::shared_ptr<FdmLinearOpComposite> map,
                                     const bc_set& bcSet = bc_set());

        void step(array_type& a, Time t);
        void setStep(Time dt);

      protected:
        friend class CrankNicolsonScheme;
        void step(array_type& a, Time t, Real theta);

        Time dt_;
        const ext::shared_ptr<FdmLinearOpComposite> map_;
        const BoundaryConditionSchemeHelper bcSet_;
    };
}

#endif


#ifndef id_609b443f171555b5bbd4c731ded2e13d
#define id_609b443f171555b5bbd4c731ded2e13d
inline bool test_609b443f171555b5bbd4c731ded2e13d(const int* i) {
    return i != nullptr;
}
#endif
