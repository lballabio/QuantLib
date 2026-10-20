/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 William Day

 This file is part of QuantLib, a free-software/open-source librarys
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

/*! \file analyticsoftbarrierengine.hpp
    \brief Analytic soft barrier european option pricing engine
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

        A soft knock-in option pays the vanilla payoff times the fraction of
        the barrier band that has been crossed, so its value is the average
        over the band of the single-barrier knock-in values. The closed form
        integrates the single-barrier formulas for down-and-in calls and
        up-and-in puts with the strike and the spot beyond the band. In all
        other cases (down-and-in puts, up-and-in calls, strikes inside or on
        the other side of the band, spot inside the band) the engine
        integrates the single-barrier formulas of AnalyticBarrierEngine over
        the band numerically. Knock-out values follow from in-out parity.
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
                    Option::Type optionType, Barrier::Type barrierType,
                    Real sigma) const;


        // Helper methods to extract parameters
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
        Real standardBarrierEquivalent() const; // for when U=L
        Real knockInValue(Real S, Real X, Rate r, Volatility sigma, Time T, Real U, Real L, Real b, Option::Type optionType, Integer eta) const;
        // Knock-in value as the average of the single-barrier knock-in values over the band
        Real bandAverageKnockInValue(Real S, Real X, Real U, Real L, Barrier::Type barrierType) const;

    };

}

#endif