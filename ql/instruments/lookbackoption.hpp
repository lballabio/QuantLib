/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou
 Copyright (C) 2007 StatPro Italia srl

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

namespace QuantLib {

    //! Continuous-floating lookback option
    /*! \ingroup instruments */
    class ContinuousFloatingLookbackOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        ContinuousFloatingLookbackOption(
                          Real currentMinmax,
                          const boost::shared_ptr<TypePayoff>& payoff,
                          const boost::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const;
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
                          const boost::shared_ptr<StrikedTypePayoff>& payoff,
                          const boost::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const;
      protected:
        // arguments
        Real minmax_;
    };

    //! %Arguments for continuous floating lookback option calculation
    class ContinuousFloatingLookbackOption::arguments
        : public OneAssetOption::arguments {
      public:
        Real minmax;
        void validate() const;
    };

    //! %Arguments for continuous fixed lookback option calculation
    class ContinuousFixedLookbackOption::arguments
        : public OneAssetOption::arguments {
      public:
        Real minmax;
        void validate() const;
    };

    //! %Continuous floating lookback %engine base class
    class ContinuousFloatingLookbackOption::engine
        : public GenericEngine<ContinuousFloatingLookbackOption::arguments,
                               ContinuousFloatingLookbackOption::results> {};

    //! %Continuous fixed lookback %engine base class
    class ContinuousFixedLookbackOption::engine
        : public GenericEngine<ContinuousFixedLookbackOption::arguments,
                               ContinuousFixedLookbackOption::results> {};

}


#endif
