
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

/*! \file option.hpp
    \brief Base option class

    \fullpath
    ql/%option.hpp
*/

// $Id$

#ifndef quantlib_option_h
#define quantlib_option_h

#include <ql/instrument.hpp>
#include <ql/argsandresults.hpp>
#include <ql/handle.hpp>
#include <ql/null.hpp>

namespace QuantLib {

    class OptionPricingEngine;

    //! base option class
    class Option : public Instrument {
      public:
        enum Type { Call, Put, Straddle };
        Option(const Handle<OptionPricingEngine>& engine,
               const std::string& isinCode = "",
               const std::string& description = "");
        virtual ~Option();
        void setPricingEngine(const Handle<OptionPricingEngine>& engine) {
            engine_ = engine;
            setupEngine();
        }
      protected:
        virtual void setupEngine() const = 0;
        virtual void performCalculations() const;
        Handle<OptionPricingEngine> engine_;
    };

    //! %option pricing results
    class OptionValue : public virtual Results {
      public:
        OptionValue() : value(Null<double>()) {}
        double value;
    };

    //! %option pricing results
    class OptionGreeks : public virtual Results {
      public:
        OptionGreeks() : delta(Null<double>()), gamma(Null<double>()),
                         theta(Null<double>()), vega(Null<double>()),
                         rho(Null<double>()), dividendRho(Null<double>()) {}
        double delta, gamma;
        double theta;
        double vega;
        double rho, dividendRho;
    };


    //! base class for option pricing engines
    class OptionPricingEngine {
      public:
        virtual ~OptionPricingEngine() {}
        virtual Arguments* parameters() = 0;
        virtual void validateParameters() const = 0;
        virtual void calculate() const = 0;
        virtual const Results* results() const = 0;
    };

}


#endif
