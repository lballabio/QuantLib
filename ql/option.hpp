

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
        void setPricingEngine(const Handle<OptionPricingEngine>&);
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
