/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Yue Tian

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

/*! \file doublebarrieroption.hpp
    \brief Double-barrier option on a single asset
*/

#ifndef quantlib_double_barrier_option_hpp
#define quantlib_double_barrier_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/barriertype.hpp>
#include <ql/instruments/payoffs.hpp>

#include <vector>

namespace QuantLib {

    class GeneralizedBlackScholesProcess;

    //! %Barrier option on a single asset.
    /*! The analytic pricing engine will be used if none if passed.

        \ingroup instruments
    */
    class DoubleBarrierOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        DoubleBarrierOption(std::vector<Barrier::Type> barrierType,
                      std::vector<Real> barrier,
                      std::vector<Real> rebate,
                      const boost::shared_ptr<StrikedTypePayoff>& payoff,
                      const boost::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const;
        /*! \warning see VanillaOption for notes on implied-volatility
                     calculation.
        */
        Volatility impliedVolatility(
             Real price,
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Real accuracy = 1.0e-4,
             Size maxEvaluations = 100,
             Volatility minVol = 1.0e-7,
             Volatility maxVol = 4.0) const;
      protected:
        // arguments
        std::vector<Barrier::Type> barrierType_;
        std::vector<Real> barrier_;
        std::vector<Real> rebate_;
    };

    //! %Arguments for barrier option calculation
    class DoubleBarrierOption::arguments : public OneAssetOption::arguments {
      public:
        arguments();
        std::vector<Barrier::Type> barrierType;
        std::vector<Real> barrier;
        std::vector<Real> rebate;
        void validate() const;
    };

    //! %Barrier-option %engine base class
    class DoubleBarrierOption::engine
        : public GenericEngine<DoubleBarrierOption::arguments,
                               DoubleBarrierOption::results> {
      protected:
        bool triggered(Real underlying) const;
    };

}

#endif
