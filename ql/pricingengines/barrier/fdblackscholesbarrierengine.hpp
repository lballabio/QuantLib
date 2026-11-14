/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen
 Copyright (C) 2026 Jogi Sidhu

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

/*! \file fdblackscholesbarrierengine.hpp
    \brief Finite-differences Black/Scholes barrier-option engine
*/

#ifndef quantlib_fd_black_scholes_barrier_engine_hpp
#define quantlib_fd_black_scholes_barrier_engine_hpp

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/instruments/barrieroption.hpp>

namespace QuantLib {

    //! Finite-differences Black/Scholes barrier-option engine
    /*! \ingroup barrierengines

        American and Bermudan exercise are supported.  In-out parity does
        not hold with early exercise, so such knock-in options are priced
        with two solves: the vanilla option received on knock-in, and the
        option not yet knocked in, which cannot be exercised and takes the
        value of the former on the barrier as its boundary condition.  A
        dividend that takes the underlying through the barrier of a
        down-and-in option also knocks it in.

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
              and comparison with Black pricing.
    */
    class FdBlackScholesBarrierEngine : public BarrierOption::engine {
      public:
        explicit FdBlackScholesBarrierEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>());

        explicit FdBlackScholesBarrierEngine(
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
        void calculateEarlyExerciseKnockIn() const;

        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        DividendSchedule dividends_;
        Size tGrid_, xGrid_, dampingSteps_;
        FdmSchemeDesc schemeDesc_;
        bool localVol_;
        Real illegalLocalVolOverwrite_;
    };

}

#endif
