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

/*! \file fdmbackwardsolver.hpp
*/

#ifndef quantlib_fdm_backward_solver_hpp
#define quantlib_fdm_backward_solver_hpp

#include <ql/methods/finitedifferences/utilities/fdmboundaryconditionset.hpp>

namespace QuantLib {

    class FdmLinearOpComposite;
    class FdmStepConditionComposite;

    struct FdmSchemeDesc {
        enum FdmSchemeType { HundsdorferType, DouglasType,
                             CraigSneydType, ModifiedCraigSneydType, 
                             ImplicitEulerType, ExplicitEulerType,
                             MethodOfLinesType, TrBDF2Type,
                             CrankNicolsonType };

        FdmSchemeDesc(FdmSchemeType type, Real theta, Real mu);

        const FdmSchemeType type;
        const Real theta, mu;

        // some default scheme descriptions
        static FdmSchemeDesc Douglas(); //same as Crank-Nicolson in 1 dimension
        static FdmSchemeDesc CrankNicolson();
        static FdmSchemeDesc ImplicitEuler();
        static FdmSchemeDesc ExplicitEuler();
        static FdmSchemeDesc CraigSneyd();
        static FdmSchemeDesc ModifiedCraigSneyd(); 
        static FdmSchemeDesc Hundsdorfer();
        static FdmSchemeDesc ModifiedHundsdorfer();
        static FdmSchemeDesc MethodOfLines(
            Real eps=0.001, Real relInitStepSize=0.01);
        static FdmSchemeDesc TrBDF2();
    };
        
    class FdmBackwardSolver {
      public:
        typedef FdmLinearOp::array_type array_type;

        FdmBackwardSolver(ext::shared_ptr<FdmLinearOpComposite> map,
                          FdmBoundaryConditionSet bcSet,
                          const ext::shared_ptr<FdmStepConditionComposite>& condition,
                          const FdmSchemeDesc& schemeDesc);

        void rollback(array_type& a, 
                      Time from, Time to,
                      Size steps, Size dampingSteps);

      protected:
        const ext::shared_ptr<FdmLinearOpComposite> map_;
        const FdmBoundaryConditionSet bcSet_;
        const ext::shared_ptr<FdmStepConditionComposite> condition_;
        const FdmSchemeDesc schemeDesc_;
    };
}

#endif


#ifndef id_616e04830209ee3d69f0c1669b3630bc
#define id_616e04830209ee3d69f0c1669b3630bc
inline bool test_616e04830209ee3d69f0c1669b3630bc(int* i) { return i != 0; }
#endif
