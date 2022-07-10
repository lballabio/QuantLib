/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Klaus Spanderen

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

/*! \file qrplusamericanengine.hpp
*/

#ifndef quantlib_qr_plus_american_engine_hpp
#define quantlib_qr_plus_american_engine_hpp

#include <ql/utilities/null.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>


namespace QuantLib {
    class QrPlusBoundaryEvaluator;

    class QrPlusAmericanEngine : public VanillaOption::engine {
      public:
        struct PutOptionParam {
            Real S, K;
            Rate r, q;
            Volatility vol;
            Time T;
        };
        enum SolverType {Brent, Newton, Ridder, Halley, SuperHalley};

        explicit QrPlusAmericanEngine(
            const ext::shared_ptr<GeneralizedBlackScholesProcess>&,
            Size interpolationPoints = 8,
            SolverType solverType = Halley,
            Real eps = 1e-6,
            Size maxIter = Null<Size>());

        void calculate() const override;

        std::pair<Size, Real> putExerciseBoundary(
            const PutOptionParam& param, Time tau) const;

      private:
        template <class Solver>
        Real buildInSolver(
            const QrPlusBoundaryEvaluator& eval,
            Solver solver, Real S, Real strike, Size maxIter,
            Real guess = Null<Real>()) const;

        Real calculate_put(
            Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const;

        const ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        const Size interpolationPoints_;
        const SolverType solverType_;
        const Real eps_;
        const Size maxIter_;
    };
}

#endif
