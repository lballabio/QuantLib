

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file swaption.hpp
    \brief Swaption class

    \fullpath
    ql/Instruments/%swaption.hpp
*/

// $Id$

#ifndef quantlib_instruments_swaption_h
#define quantlib_instruments_swaption_h

#include <ql/exercise.hpp>
#include <ql/Instruments/simpleswap.hpp>
#include <ql/InterestRateModelling/model.hpp>

namespace QuantLib {

    namespace Instruments {

        class SwaptionParameters;

        //! Swaption class
        class Swaption : public Option {
          public:
            Swaption(const SimpleSwap& swap,
                     const Exercise& exercise,
                     const RelinkableHandle<TermStructure>& termStructure,
                     const Handle<OptionPricingEngine>& engine);
            ~Swaption();
          protected:
            void setupEngine() const;
          private:
            // parameters
            const SimpleSwap& swap_;
            Exercise exercise_;
            const RelinkableHandle<TermStructure>& termStructure_;
            // helper class for implied volatility calculation
        };

        //! parameters for swaption calculation
        class SwaptionParameters : public virtual Arguments {
          public:
            SwaptionParameters() : payFixed(false),
                                   fixedPayTimes(0),
                                   fixedCoupons(0),
                                   floatingResetTimes(0),
                                   floatingPayTimes(0),
                                   nominals(0),
                                   exerciseType(Exercise::European), 
                                   exerciseTimes(0) {}
            bool payFixed;
            std::vector<Time> fixedPayTimes;
            std::vector<double> fixedCoupons;
            std::vector<Time> floatingResetTimes;
            std::vector<Time> floatingPayTimes;
            std::vector<double> nominals;
            Exercise::Type exerciseType;
            std::vector<Time> exerciseTimes;
        };

        //! %results from swaption calculation
        class SwaptionResults : public OptionValue {};

    }

    namespace Pricers {

        //! base class for swaption pricing engines
        /*! Derived engines only need to implement the <tt>calculate()</tt>
            method
        */
        class SwaptionPricingEngine : public OptionPricingEngine {
          public:
            Arguments* parameters();
            void validateParameters() const;
            const Results* results() const;
            void setModel(const Handle<InterestRateModelling::Model>& model) {
                model_ = model;
            }
          protected:
            Instruments::SwaptionParameters parameters_;
            mutable Instruments::SwaptionResults results_;
            Handle<InterestRateModelling::Model> model_;
        };

    }

}


#endif

