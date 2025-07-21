/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008, 2009 Klaus Spanderen

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

/*! \file fdblackscholesrebateengine.hpp
    \brief Finite-differences Black/Scholes barrier option rebate helper engine
*/

#ifndef quantlib_fd_black_scholes_rebate_engine_hpp
#define quantlib_fd_black_scholes_rebate_engine_hpp

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/instruments/barrieroption.hpp>

namespace QuantLib {

    //! Finite-differences Black/Scholes barrier-option rebate helper engine
    /*! \ingroup barrierengines */
    class FdBlackScholesRebateEngine : public BarrierOption::engine {
      public:
        explicit FdBlackScholesRebateEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>());

        explicit FdBlackScholesRebateEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process,
            DividendSchedule dividends,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>());

        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        DividendSchedule dividends_;
        Size tGrid_, xGrid_, dampingSteps_;
        FdmSchemeDesc schemeDesc_;
        bool localVol_;
        Real illegalLocalVolOverwrite_;
    };

}

#endif
