
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
/*! \file vanillaoptionengine.hpp
    \brief Vanilla (no discrete dividends, no barriers) option on a single asset

    \fullpath
    ql/Instruments/%vanillaoptionengine.hpp
*/

// $Id$

#ifndef quantlib_vanillaoptionengine_h
#define quantlib_vanillaoptionengine_h

#include <ql/option.hpp>
#include <ql/types.hpp>
#include <ql/argsandresults.hpp>
#include <ql/pricingengine.hpp>


namespace QuantLib {

    namespace Pricers {

        //! parameters for vanilla option calculation
        class VanillaOptionParameters : public virtual Arguments {
          public:
            VanillaOptionParameters() : type(Option::Type(-1)),
                                      underlying(Null<double>()),
                                      strike(Null<double>()),
                                      dividendYield(Null<double>()),
                                      riskFreeRate(Null<double>()),
                                      residualTime(Null<double>()),
                                      volatility(Null<double>()) {}
            Option::Type type;
            double underlying, strike;
            Spread dividendYield;
            Rate riskFreeRate;
            Time residualTime;
            double volatility;
        };

        //! %results from vanilla option calculation
        class VanillaOptionResults : public OptionValue, public OptionGreeks {};

        //! base class for vanilla option pricing engines
        /*! Derived engines only need to implement the <tt>calculate()</tt>
            method
        */
        class VanillaOptionEngine : public PricingEngine {
          public:
            Arguments* parameters();
            void validateParameters() const;
            const Results* results() const;
          protected:
            VanillaOptionParameters parameters_;
            mutable VanillaOptionResults results_;
        };

    }

}

#endif

