/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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

/*! \file fdndimblackscholesvanillaengine.hpp
    \brief Finite-Differences n-dimensional Black-Scholes vanilla option engine
*/

#ifndef quantlib_fd_ndim_black_scholes_vanilla_engine_hpp
#define quantlib_fd_ndim_black_scholes_vanilla_engine_hpp

#include <ql/math/matrix.hpp>
#include <ql/instruments/basketoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class GeneralizedBlackScholesProcess;

    //! n-dimensional finite-differences Black Scholes vanilla option engine

    /*! \ingroup basketengines

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
              and comparison with the PyFENG python package.
    */
    class FdndimBlackScholesVanillaEngine : public BasketOption::engine {
      public:
        FdndimBlackScholesVanillaEngine(
            std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes,
            Matrix rho,
            std::vector<Size> xGrids,
            Size tGrid = 50, Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas());


        // Auto-scaling of grids, larges eigenvalue gets xGrid size.
        FdndimBlackScholesVanillaEngine(
            std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes,
            Matrix rho,
            Size xGrid, Size tGrid = 50, Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas());

        void calculate() const override;

      private:
        const std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes_;
        const Matrix rho_;
        const std::vector<Size> xGrids_;
        const Size tGrid_, dampingSteps_;
        const FdmSchemeDesc schemeDesc_;
    };
}

#endif
