/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

/*! \file fdmsolverdesc.hpp
*/

#ifndef quantlib_fdm_solver_desc_hpp
#define quantlib_fdm_solver_desc_hpp

#include <ql/methods/finitedifferences/utilities/fdmboundaryconditionset.hpp>

namespace QuantLib {

    class FdmMesher;
    class FdmInnerValueCalculator;
    class FdmStepConditionComposite;
    class FdmInnerValueCalculator;

    struct FdmSolverDesc {
        const std::shared_ptr<FdmMesher> mesher;
        const FdmBoundaryConditionSet bcSet;
        const std::shared_ptr<FdmStepConditionComposite> condition;
        const std::shared_ptr<FdmInnerValueCalculator> calculator;
        const Time maturity;
        const Size timeSteps;
        const Size dampingSteps;
    };
}

#endif
