/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Klaus Spanderen

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

/*! \file fdblackscholesshoutengine.hpp
    \brief Finite-Differences Black Scholes shout option engine
*/

#ifndef quantlib_fd_black_scholes_shout_engine_hpp
#define quantlib_fd_black_scholes_shout_engine_hpp

#include <ql/pricingengine.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class GeneralizedBlackScholesProcess;

    class FdBlackScholesShoutEngine : public VanillaOption::engine {
      public:
        // Constructor
        explicit FdBlackScholesShoutEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas());

        FdBlackScholesShoutEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            DividendSchedule dividends,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas());

        void calculate() const override;

      private:
        const ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        DividendSchedule dividends_;
        const Size tGrid_, xGrid_, dampingSteps_;
        const FdmSchemeDesc schemeDesc_;
    };

}

#endif
