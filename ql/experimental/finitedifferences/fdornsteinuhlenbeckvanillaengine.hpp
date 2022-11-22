/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

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

/*! \file fdblackscholesvanillaengine.hpp
    \brief Finite-Differences Black Scholes vanilla option engine
*/

#ifndef quantlib_fd_ornstein_uhlenbeck_vanilla_engine_hpp
#define quantlib_fd_ornstein_uhlenbeck_vanilla_engine_hpp

#include <ql/pricingengine.hpp>
#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class YieldTermStructure;
    class OrnsteinUhlenbeckProcess;

    class FdOrnsteinUhlenbeckVanillaEngine
         : public DividendVanillaOption::engine {
      public:
        // Constructor
        FdOrnsteinUhlenbeckVanillaEngine(
            ext::shared_ptr<OrnsteinUhlenbeckProcess>,
            const ext::shared_ptr<YieldTermStructure>& rTS,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            Real epsilon = 0.0001,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas());

        void calculate() const override;

      private:
        const ext::shared_ptr<OrnsteinUhlenbeckProcess> process_;
        const ext::shared_ptr<YieldTermStructure> rTS_;
        const Size tGrid_, xGrid_, dampingSteps_;
        const Real epsilon_;
        const FdmSchemeDesc schemeDesc_;
    };
}

#endif
