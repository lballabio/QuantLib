/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Klaus Spanderen

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

/*! \file qdplusamericanengine.hpp
*/

#ifndef quantlib_qd_plus_american_engine_hpp
#define quantlib_qd_plus_american_engine_hpp

#include <ql/utilities/null.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>


namespace QuantLib {

    class Interpolation;
    class ChebyshevInterpolation;
    class QdPlusBoundaryEvaluator;

    namespace detail {

        class QdPutCallParityEngine: public VanillaOption::engine {
          public:
            explicit QdPutCallParityEngine(
                ext::shared_ptr<GeneralizedBlackScholesProcess> process);

            void calculate() const override;

          protected:
            virtual Real calculatePut(
                Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const = 0;

            const ext::shared_ptr<GeneralizedBlackScholesProcess> process_;

          private:
            Real calculatePutWithEdgeCases(
               Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const;
        };

        class QdPlusAddOnValue {
          public:
            QdPlusAddOnValue(Time T,
                             Real S,
                             Real K,
                             Rate r,
                             Rate q,
                             Volatility vol,
                             Real xmax,
                             ext::shared_ptr<Interpolation> q_z);

            Real operator()(Real z) const;
          private:
            const Time T_;
            const Real S_, K_, xmax_;
            const Rate r_, q_;
            const Volatility vol_;
            const ext::shared_ptr<Interpolation> q_z_;
            const CumulativeNormalDistribution Phi_;
        };
    }


    //! American engine based on the QD+ approximation to the exercise boundary.
    /*! The main purpose of this engine is to provide a good initial guess to the exercise
        boundary for the superior fixed point American engine QdFpAmericanEngine

        References:
        Li, M. (2009), “Analytical Approximations for the Critical Stock Prices
                        of American Options: A Performance Comparison,”
                        Working paper, Georgia Institute of Technology.

        https://mpra.ub.uni-muenchen.de/15018/1/MPRA_paper_15018.pdf
    */
    class QdPlusAmericanEngine: public detail::QdPutCallParityEngine {
      public:
        enum SolverType {Brent, Newton, Ridder, Halley, SuperHalley};

        explicit QdPlusAmericanEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            Size interpolationPoints = 8,
            SolverType solverType = Halley,
            Real eps = 1e-6,
            Size maxIter = Null<Size>());

        std::pair<Size, Real> putExerciseBoundaryAtTau(
            Real S, Real K, Rate r, Rate q,
            Volatility vol, Time T, Time tau) const;

        ext::shared_ptr<ChebyshevInterpolation> getPutExerciseBoundary(
            Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const;

        static Real xMax(Real K, Rate r, Rate q);

      protected:
        Real calculatePut(
            Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const override;

      private:
        template <class Solver>
        Real buildInSolver(
            const QdPlusBoundaryEvaluator& eval,
            Solver solver, Real S, Real strike, Size maxIter,
            Real guess = Null<Real>()) const;

        const Size interpolationPoints_;
        const SolverType solverType_;
        const Real eps_;
        const Size maxIter_;
    };
}

#endif
