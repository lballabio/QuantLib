
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
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
#include <ql/marketelement.hpp>
#include <ql/option.hpp>
#include <ql/termstructure.hpp>
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
            const Exercise& exercise_;
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

