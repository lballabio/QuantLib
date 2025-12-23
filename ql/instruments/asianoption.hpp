/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2004, 2007 StatPro Italia srl
 Copyright (C) 2025 Kareem Fareed

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

/*! \file asianoption.hpp
    \brief Asian option on a single asset
*/

#ifndef quantlib_asian_option_hpp
#define quantlib_asian_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/averagetype.hpp>
#include <ql/time/date.hpp>
#include <vector>

namespace QuantLib {

    //! Continuous-averaging Asian option
    /*! \ingroup instruments */
    class ContinuousAveragingAsianOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        /*! This constructor is for unseasoned (fresh) options where
            averaging has not yet started.
        */
        ContinuousAveragingAsianOption(
                Average::Type averageType,
                const ext::shared_ptr<StrikedTypePayoff>& payoff,
                const ext::shared_ptr<Exercise>& exercise);

        /*! This constructor is for seasoned options where averaging
            has already started. The start date is a contract term specifying
            when averaging began. The current average (market data) should be
            provided to the pricing engine.
        */
        ContinuousAveragingAsianOption(
                Average::Type averageType,
                Date startDate,
                const ext::shared_ptr<StrikedTypePayoff>& payoff,
                const ext::shared_ptr<Exercise>& exercise);

        void setupArguments(PricingEngine::arguments*) const override;

      protected:
        Average::Type averageType_;
        Date startDate_;
    };

    //! Discrete-averaging Asian option
    /*! \ingroup instruments */
    class DiscreteAveragingAsianOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        /*! This constructor takes the running sum or product of past fixings,
            depending on the average type.  The fixing dates passed here can be
            only the future ones.
        */
        DiscreteAveragingAsianOption(Average::Type averageType,
                                     Real runningAccumulator,
                                     Size pastFixings,
                                     std::vector<Date> fixingDates,
                                     const ext::shared_ptr<StrikedTypePayoff>& payoff,
                                     const ext::shared_ptr<Exercise>& exercise);

        /*! This constructor takes past fixings as a vector, defaulting to an empty
            vector representing an unseasoned option.  This constructor expects *all* fixing dates
            to be provided, including those in the past, and to be already sorted.  During the
            calculations, the option will compare them to the evaluation date to determine which
            are historic; it will then take as many values from allPastFixings as needed and ignore
            the others.  If not enough fixings are provided, it will raise an error.
        */
        DiscreteAveragingAsianOption(Average::Type averageType,
                                     std::vector<Date> fixingDates,
                                     const ext::shared_ptr<StrikedTypePayoff>& payoff,
                                     const ext::shared_ptr<Exercise>& exercise,
                                     std::vector<Real> allPastFixings = std::vector<Real>());

        void setupArguments(PricingEngine::arguments*) const override;

      protected:
        Average::Type averageType_;
        Real runningAccumulator_;
        Size pastFixings_;
        std::vector<Date> fixingDates_;

        // For backwards compatibility with the traditional interface, we keep track of
        // whether this option was initialised using the full array of seasoned fixings
        // (even if empty) or if a pastFixings and a runningAccumulator was provided
        bool allPastFixingsProvided_;
        std::vector<Real> allPastFixings_;
    };

    //! Extra %arguments for single-asset discrete-average Asian option
    class DiscreteAveragingAsianOption::arguments
        : public OneAssetOption::arguments {
      public:
        arguments() : averageType(Average::Type(-1)),
                      runningAccumulator(Null<Real>()),
                      pastFixings(Null<Size>()) {}
        void validate() const override;
        Average::Type averageType;
        Real runningAccumulator;
        Size pastFixings;
        std::vector<Date> fixingDates;
    };

    //! Extra %arguments for single-asset continuous-average Asian option
    class ContinuousAveragingAsianOption::arguments
        : public OneAssetOption::arguments {
      public:
        arguments() : averageType(Average::Type(-1))
                      {}
        void validate() const override;
        Average::Type averageType;
        Date startDate;
    };

    //! Discrete-averaging Asian %engine base class
    class DiscreteAveragingAsianOption::engine
        : public GenericEngine<DiscreteAveragingAsianOption::arguments,
                               DiscreteAveragingAsianOption::results> {};

    //! Continuous-averaging Asian %engine base class
    class ContinuousAveragingAsianOption::engine
        : public GenericEngine<ContinuousAveragingAsianOption::arguments,
                               ContinuousAveragingAsianOption::results> {};

}


#endif
