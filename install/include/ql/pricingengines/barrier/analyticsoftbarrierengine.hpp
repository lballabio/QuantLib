/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 William Day

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

/*! \file analyticsoftbarrierengine.hpp
    \brief Analytic soft barrier european option pricing engine

    \ingroup barrierengines

    \test correctness is verified by comparing to known benchmark values         ######## do i have stability checks????
          and stability checks.
*/

#ifndef quantlib_analytic_soft_barrier_engine_hpp
#define quantlib_analytic_soft_barrier_engine_hpp

#include <ql/instruments/softbarrieroption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    //! Pricing engine for soft barrier european options using an analytical formula
    /*! Formulas are taken from "The complete guide to option pricing formulas 2nd Ed", E.G. Haug, p.165.
        Implements a closed form solution for soft barrier options originally introduced by Hart and Ross (1994).
    */

    class AnalyticSoftBarrierEngine : public SoftBarrierOption::engine {
      public:
        explicit AnalyticSoftBarrierEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);

        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        CumulativeNormalDistribution f_;

        void validateInputs(Real S, Real X, Rate r, Rate q, Time T, Real U, Real L,
                    Option::Type optionType, SoftBarrier::Type barrierType,
                    Real sigma) const;

        // Helper methods to extract relevant parameters
        Real underlying() const;
        Real strike() const;
        Time residualTime() const;
        Volatility volatility() const;
        Real volatilitySquared() const;
        Real barrierLo() const;
        Real barrierHi() const;
        Real stdDeviation() const;
        Rate riskFreeRate() const;
        DiscountFactor riskFreeDiscount() const;
        Rate dividendYield() const; 
        Rate costOfCarry() const;  
        DiscountFactor dividendDiscount() const;

        // Pricing helpers 
        Real vanillaEquivalent() const;    // Computes the equivalent vanilla  option price (with no barriers)
        Real standardBarrierEquivalent() const; // Computes equivalent standard barrier option price when user sets upper barrier equal lower barrier
        Real knockInValue() const;

    };

}

#endif