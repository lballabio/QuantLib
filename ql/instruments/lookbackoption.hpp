/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2014 Francois Botha

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

/*! \file lookbackoption.hpp
    \brief Lookback option on a single asset
*/

#ifndef quantlib_lookback_option_hpp
#define quantlib_lookback_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    //! Continuous-floating lookback option
    /*! \ingroup instruments */
    class ContinuousFloatingLookbackOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        ContinuousFloatingLookbackOption(
                          Real currentMinmax,
                          const std::shared_ptr<TypePayoff>& payoff,
                          const std::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const override;

      protected:
        // arguments
        Real minmax_;
    };

    //! Continuous-fixed lookback option
    /*! \ingroup instruments */
    class ContinuousFixedLookbackOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        ContinuousFixedLookbackOption(
                          Real currentMinmax,
                          const std::shared_ptr<StrikedTypePayoff>& payoff,
                          const std::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const override;

      protected:
        // arguments
        Real minmax_;
    };

    //! Continuous-partial-floating lookback option
    /*! From http://help.rmetrics.org/fExoticOptions/LookbackOptions.html :

        For a partial-time floating strike lookback option, the
        lookback period starts at time zero and ends at an arbitrary
        date before expiration. Except for the partial lookback
        period, the option is similar to a floating strike lookback
        option. The partial-time floating strike lookback option is
        cheaper than a similar standard floating strike lookback
        option. Partial-time floating strike lookback options can be
        priced analytically using a model introduced by Heynen and Kat
        (1994).

        \ingroup instruments
    */
    class ContinuousPartialFloatingLookbackOption : public ContinuousFloatingLookbackOption {
      public:
        class arguments;
        class engine;
        ContinuousPartialFloatingLookbackOption(
                          Real currentMinmax,
                          Real lambda,
                          Date lookbackPeriodEnd,
                          const std::shared_ptr<TypePayoff>& payoff,
                          const std::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const override;

      protected:
        // arguments
        Real lambda_;
        Date lookbackPeriodEnd_;
    };

    //! Continuous-partial-fixed lookback option
    /*! From http://help.rmetrics.org/fExoticOptions/LookbackOptions.html :

        For a partial-time fixed strike lookback option, the lookback
        period starts at a predetermined date after the initialization
        date of the option.  The partial-time fixed strike lookback
        call option payoff is given by the difference between the
        maximum observed price of the underlying asset during the
        lookback period and the fixed strike price. The partial-time
        fixed strike lookback put option payoff is given by the
        difference between the fixed strike price and the minimum
        observed price of the underlying asset during the lookback
        period. The partial-time fixed strike lookback option is
        cheaper than a similar standard fixed strike lookback
        option. Partial-time fixed strike lookback options can be
        priced analytically using a model introduced by Heynen and Kat
        (1994).

        \ingroup instruments
    */
    class ContinuousPartialFixedLookbackOption : public ContinuousFixedLookbackOption {
      public:
        class arguments;
        class engine;
        ContinuousPartialFixedLookbackOption(
                          Date lookbackPeriodStart,
                          const std::shared_ptr<StrikedTypePayoff>& payoff,
                          const std::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const override;

      protected:
        // arguments
        Date lookbackPeriodStart_;
    };

    //! %Arguments for continuous floating lookback option calculation
    class ContinuousFloatingLookbackOption::arguments
        : public OneAssetOption::arguments {
      public:
        Real minmax;
        void validate() const override;
    };

    //! %Arguments for continuous fixed lookback option calculation
    class ContinuousFixedLookbackOption::arguments
        : public OneAssetOption::arguments {
      public:
        Real minmax;
        void validate() const override;
    };

    //! %Arguments for continuous partial floating lookback option calculation
    class ContinuousPartialFloatingLookbackOption::arguments
        : public ContinuousFloatingLookbackOption::arguments {
      public:
        Real lambda;
        Date lookbackPeriodEnd;
        void validate() const override;
    };

    //! %Arguments for continuous partial fixed lookback option calculation
    class ContinuousPartialFixedLookbackOption::arguments
        : public ContinuousFixedLookbackOption::arguments {
      public:
        Date lookbackPeriodStart;
        void validate() const override;
    };

    //! %Continuous floating lookback %engine base class
    class ContinuousFloatingLookbackOption::engine
        : public GenericEngine<ContinuousFloatingLookbackOption::arguments,
                               ContinuousFloatingLookbackOption::results> {};

    //! %Continuous fixed lookback %engine base class
    class ContinuousFixedLookbackOption::engine
        : public GenericEngine<ContinuousFixedLookbackOption::arguments,
                               ContinuousFixedLookbackOption::results> {};

    //! %Continuous partial floating lookback %engine base class
    class ContinuousPartialFloatingLookbackOption::engine
        : public GenericEngine<ContinuousPartialFloatingLookbackOption::arguments,
                               ContinuousPartialFloatingLookbackOption::results> {};

    //! %Continuous partial fixed lookback %engine base class
    class ContinuousPartialFixedLookbackOption::engine
        : public GenericEngine<ContinuousPartialFixedLookbackOption::arguments,
                               ContinuousPartialFixedLookbackOption::results> {};
}


#endif
