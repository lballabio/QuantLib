/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Neil Firth
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003, 2004, 2007 StatPro Italia srl

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

/*! \file barrieroption.hpp
    \brief Barrier option on a single asset
*/

#ifndef quantlib_barrier_option_hpp
#define quantlib_barrier_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/barriertype.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/dividendschedule.hpp>

namespace QuantLib {

    class GeneralizedBlackScholesProcess;

    //! %Barrier option on a single asset.
    /*! The analytic pricing engine will be used if none if passed.

        \ingroup instruments
    */
    class BarrierOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        BarrierOption(Barrier::Type barrierType,
                      Real barrier,
                      Real rebate,
                      const ext::shared_ptr<StrikedTypePayoff>& payoff,
                      const ext::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const override;

        /*! \warning see VanillaOption for notes on implied-volatility
                     calculation.
        */
        //@{
        Volatility impliedVolatility(
             Real price,
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Real accuracy = 1.0e-4,
             Size maxEvaluations = 100,
             Volatility minVol = 1.0e-7,
             Volatility maxVol = 4.0) const;
        Volatility impliedVolatility(
             Real price,
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             const DividendSchedule& dividends,
             Real accuracy = 1.0e-4,
             Size maxEvaluations = 100,
             Volatility minVol = 1.0e-7,
             Volatility maxVol = 4.0) const;
        //@}
      protected:
        // arguments
        Barrier::Type barrierType_;
        Real barrier_;
        Real rebate_;
    };

    //! %Arguments for barrier option calculation
    class BarrierOption::arguments : public OneAssetOption::arguments {
      public:
        arguments();
        Barrier::Type barrierType;
        Real barrier;
        Real rebate;
        void validate() const override;
    };

    //! %Barrier-option %engine base class
    class BarrierOption::engine
        : public GenericEngine<BarrierOption::arguments,
                               BarrierOption::results> {
      protected:
        bool triggered(Real underlying) const;
    };

}

#endif
