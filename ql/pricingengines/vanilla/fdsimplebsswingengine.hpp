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

/*! \file fdsimplebsswingengine.hpp
    \brief Finite Differences Black-Scholes engine for simple swing options
*/

#ifndef quantlib_fd_simple_bs_swing_engine_hpp
#define quantlib_fd_simple_bs_swing_engine_hpp

#include <ql/pricingengine.hpp>
#include <ql/instruments/vanillaswingoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    //! Finite-Differences Black Scholes engine for simple swing options

    class GeneralizedBlackScholesProcess;

    class FdSimpleBSSwingEngine
        : public GenericEngine<VanillaSwingOption::arguments,
                               VanillaSwingOption::results> {
      public:
        explicit FdSimpleBSSwingEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> p,
                                       Size tGrid = 50,
                                       Size xGrid = 100,
                                       const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas());

        void calculate() const override;

      private:
        const ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        const Size tGrid_, xGrid_;
        const FdmSchemeDesc schemeDesc_;
    };
}

#endif
