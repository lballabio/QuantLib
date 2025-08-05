/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 William Day

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers – http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license. You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the license for more details.
*/

/*! \file softbarrieroption.hpp
    \brief Soft Barrier european option on a single asset
*/

#ifndef quantlib_soft_barrier_option_hpp
#define quantlib_soft_barrier_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/barriertype.hpp>

namespace QuantLib {

    class GeneralizedBlackScholesProcess;

    //! Soft barrier option on a single asset
    /*! A soft barrier option gets knocked in/out proportionally over the barrier range instead of being knocked in/out in full at a hard barrier.
        It is currently only available with European payoff stylet

        \ingroup instruments
    */
    class SoftBarrierOption : public OneAssetOption {
      public:
        class arguments;
        class engine;

        SoftBarrierOption(Barrier::Type barrierType,
                          Real barrier_lo,
                          Real barrier_hi,
                          const ext::shared_ptr<StrikedTypePayoff>& payoff,
                          const ext::shared_ptr<Exercise>& exercise);

        void setupArguments(PricingEngine::arguments*) const override;

        Volatility impliedVolatility(
             Real price,
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Real accuracy = 1.0e-4,
             Size maxEvaluations = 100,
             Volatility minVol = 1e-6, // 0 vol values can make the soft barrier formula produce NaN results
             Volatility maxVol = 4.0) const;

      private:
        Barrier::Type barrierType_;
        Real barrier_lo_;
        Real barrier_hi_;
    };

    // Arguments for soft barrier option calc
    class SoftBarrierOption::arguments : public OneAssetOption::arguments {
      public:
        arguments();
        Barrier::Type barrierType;
        Real barrier_lo;
        Real barrier_hi;
    };

    // Base class for soft barrier option engines
    class SoftBarrierOption::engine
        : public GenericEngine<SoftBarrierOption::arguments,
                               SoftBarrierOption::results> {};

}

#endif