/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
#include <ql/numericalmethod.hpp>
#include <ql/option.hpp>
#include <ql/Instruments/simpleswap.hpp>

namespace QuantLib {

    namespace Instruments {

        class SwaptionParameters;

        //! Swaption class
        class Swaption : public Option {
          public:
            Swaption(const Handle<SimpleSwap>& swap,
                     const Exercise& exercise,
                     const RelinkableHandle<TermStructure>& termStructure,
                     const Handle<OptionPricingEngine>& engine);
          protected:
            void performCalculations() const;
            void setupEngine() const;
          private:
            // parameters
            Handle<SimpleSwap> swap_;
            Exercise exercise_;
            const RelinkableHandle<TermStructure>& termStructure_;
        };

        //! parameters for swaption calculation
        class SwaptionParameters : public virtual Arguments {
          public:
            SwaptionParameters() : payFixed(false),
                                   fairRate(0.0),
                                   fixedRate(0.0),
                                   fixedBPS(0.0),
                                   fixedPayTimes(0),
                                   fixedCoupons(0),
                                   floatingResetTimes(0),
                                   floatingPayTimes(0),
                                   nominal(0.0),
                                   exerciseType(Exercise::Type(-1)),
                                   exerciseTimes(0) {}
            bool payFixed;
            Rate fairRate;
            Rate fixedRate;
            double fixedBPS;
            std::vector<Time> fixedPayTimes;
            std::vector<double> fixedCoupons;
            std::vector<Time> floatingResetTimes;
            std::vector<Time> floatingPayTimes;
            double nominal;
            Exercise::Type exerciseType;
            std::vector<Time> exerciseTimes;
            void validate() const;
        };

        //! %results from swaption calculation
        class SwaptionResults : public OptionValue {};

    }

}


#endif

