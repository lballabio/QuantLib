
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file asianoption.hpp
    \brief Asian option on a single asset
*/

#ifndef quantlib_asian_option_hpp
#define quantlib_asian_option_hpp

#include <ql/Instruments/oneassetstrikedoption.hpp>

namespace QuantLib {

    //! placeholder for enumerated averaging types
    struct Average {
        enum Type { Arithmetic, Geometric };
    };

    //! Continuous-averaging Asian option
    /*! \todo add running average

        \ingroup instruments
    */
    class ContinuousAveragingAsianOption : public OneAssetStrikedOption {
      public:
        class arguments;
        class engine;
        ContinuousAveragingAsianOption(
                Average::Type averageType,
                const boost::shared_ptr<StochasticProcess>&,
                const boost::shared_ptr<StrikedTypePayoff>& payoff,
                const boost::shared_ptr<Exercise>& exercise,
                const boost::shared_ptr<PricingEngine>& engine =
                                           boost::shared_ptr<PricingEngine>());
        void setupArguments(Arguments*) const;
      protected:
        Average::Type averageType_;
    };

    //! Discrete-averaging Asian option
    /*! \ingroup instruments */
    class DiscreteAveragingAsianOption : public OneAssetStrikedOption {
      public:
        class arguments;
        class engine;
        DiscreteAveragingAsianOption(
                Average::Type averageType,
                Real runningAccumulator,
                Size pastFixings,
                std::vector<Date> fixingDates,
                const boost::shared_ptr<StochasticProcess>&,
                const boost::shared_ptr<StrikedTypePayoff>& payoff,
                const boost::shared_ptr<Exercise>& exercise,
                const boost::shared_ptr<PricingEngine>& engine =
                                           boost::shared_ptr<PricingEngine>());
        void setupArguments(Arguments*) const;
      protected:
        Average::Type averageType_;
        Real runningAccumulator_;
        Size pastFixings_;
        std::vector<Date> fixingDates_;
    };

    //! Extra arguments for single-asset discrete-average Asian option
    class DiscreteAveragingAsianOption::arguments
        : public OneAssetStrikedOption::arguments {
      public:
        arguments() : averageType(Average::Type(-1)),
                      runningAccumulator(Null<Real>()),
                      pastFixings(Null<Size>()) {}
        void validate() const;
        Average::Type averageType;
        Real runningAccumulator;
        Size pastFixings;
        std::vector<Date> fixingDates;
    };

    //! Extra arguments for single-asset continuous-average Asian option
    class ContinuousAveragingAsianOption::arguments
        : public OneAssetStrikedOption::arguments {
      public:
        arguments() : averageType(Average::Type(-1)) {}
        void validate() const;
        Average::Type averageType;
    };

    //! Discrete-averaging Asian engine base class
    class DiscreteAveragingAsianOption::engine
        : public GenericEngine<DiscreteAveragingAsianOption::arguments,
                               DiscreteAveragingAsianOption::results> {};

    //! Continuous-averaging Asian engine base class
    class ContinuousAveragingAsianOption::engine
        : public GenericEngine<ContinuousAveragingAsianOption::arguments,
                               ContinuousAveragingAsianOption::results> {};

}


#endif

