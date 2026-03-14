/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Thema Consulting SA

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

/*! \file doublebarrieroption.hpp
    \brief double Barrier european option on a single asset
*/

#ifndef quantlib_double_barrier_option_hpp
#define quantlib_double_barrier_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/doublebarriertype.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    class GeneralizedBlackScholesProcess;

    //! %Double Barrier option on a single asset.
    /*! The analytic pricing engine will be used if none if passed.

        \ingroup instruments
    */
    class DoubleBarrierOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        DoubleBarrierOption(DoubleBarrier::Type barrierType,
                      Real barrier_lo,
                      Real barrier_hi,
                      Real rebate,
                      const ext::shared_ptr<StrikedTypePayoff>& payoff,
                      const ext::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const override;
        /*! \warning see VanillaOption for notes on implied-volatility
                     calculation.
        */
        Volatility impliedVolatility(
             Real price,
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Real accuracy = 1.0e-4,
             Size maxEvaluations = 100,
             Volatility minVol = 1.0e-7,
             Volatility maxVol = 4.0) const;
      protected:
        // arguments
        DoubleBarrier::Type barrierType_;
        Real barrier_lo_;
        Real barrier_hi_;
        Real rebate_;
    };

    //! %Arguments for double barrier option calculation
    class DoubleBarrierOption::arguments : public OneAssetOption::arguments {
      public:
        arguments();
        DoubleBarrier::Type barrierType;
        Real barrier_lo;
        Real barrier_hi;
        Real rebate;
        void validate() const override;
    };

    //! %Double-Barrier-option %engine base class
    class DoubleBarrierOption::engine
        : public GenericEngine<DoubleBarrierOption::arguments,
                               DoubleBarrierOption::results> {
      protected:
        bool triggered(Real underlying) const;
    };

}

#endif
