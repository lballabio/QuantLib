/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2019 Klaus Spanderen

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

/*! \file fdsabrvanillaengine.hpp
    \brief Finite-Differences pricing engine for the SABR model
*/

#ifndef quantlib_fd_sabr_vanilla_engine_hpp
#define quantlib_fd_sabr_vanilla_engine_hpp

#include <ql/handle.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {
    class YieldTermStructure;

    class FdSabrVanillaEngine : public VanillaOption::engine {
      public:
        FdSabrVanillaEngine(Real f0,
                            Real alpha,
                            Real beta,
                            Real nu,
                            Real rho,
                            Handle<YieldTermStructure> rTS,
                            Size tGrid = 50,
                            Size fGrid = 400,
                            Size xGrid = 50,
                            Size dampingSteps = 0,
                            Real scalingFactor = 1.0,
                            Real eps = 1e-4,
                            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

        void calculate() const override;

      private:
        const Real f0_, alpha_, beta_, nu_, rho_;
        const Handle<YieldTermStructure> rTS_;
        const Size tGrid_, fGrid_, xGrid_, dampingSteps_;
        const Real scalingFactor_, eps_;
        const FdmSchemeDesc schemeDesc_;
    };
}

#endif
