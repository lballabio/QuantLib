
/*
 Copyright (C) 2002 Ferdinando Ametrano
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
/*! \file genericengine.hpp
    \brief Generic option engine

    \fullpath
    ql/PricingEngines/%genericengine.hpp
*/

// $Id$

#ifndef quantlib_generic_engine_h
#define quantlib_generic_engine_h

#include <ql/option.hpp>
#include <ql/types.hpp>
#include <ql/argsandresults.hpp>
#include <ql/pricingengine.hpp>


namespace QuantLib {

    namespace PricingEngines {

        //! template base class for option pricing engines
        /*! Derived engines only need to implement the <tt>calculate()</tt>
            method the inherit from PricingEngine
        */
        template<class ArgumentsType, class ResultsType>
        class GenericEngine : public PricingEngine {
          public:
            Arguments* parameters() { return &parameters_; }
            const Results* results() const { return &results_; }
          protected:
            ArgumentsType parameters_;
            mutable ResultsType results_;
        };


        
        
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
            void validate() const;
            Option::Type type;
            double underlying, strike;
            Spread dividendYield;
            Rate riskFreeRate;
            Time residualTime;
            double volatility;
        };

        inline void VanillaOptionParameters::validate() const {
            QL_REQUIRE(type != Option::Type(-1),
                       "no option type given");
            QL_REQUIRE(underlying != Null<double>(),
                       "null underlying given");
            QL_REQUIRE(underlying > 0.0,
                       "negative or zero underlying given");
            QL_REQUIRE(strike != Null<double>(),
                       "null strike given");
            QL_REQUIRE(strike > 0.0,
                       "negative or zero strike given");
            QL_REQUIRE(dividendYield != Null<double>(),
                       "null dividend yield given");
            QL_REQUIRE(riskFreeRate != Null<double>(),
                       "null risk free rate given");
            QL_REQUIRE(residualTime != Null<double>(),
                       "null residual time given");
            QL_REQUIRE(residualTime >= 0.0,
                       "negative residual time given");
            QL_REQUIRE(volatility != Null<double>(),
                       "null volatility given");
            QL_REQUIRE(volatility >= 0.0,
                       "negative volatility given");
        }

        //! %results from vanilla option calculation
        class VanillaOptionResults : public OptionValue, public OptionGreeks {};


        
        
        
        //! parameters for quanto option calculation
        class QuantoOptionParameters : public VanillaOptionParameters {
          public:
            QuantoOptionParameters() : VanillaOptionParameters(),
                                       foreignRiskFreeRate(Null<double>()),
                                       exchangeRateVolatility(Null<double>()),
                                       correlation(Null<double>()) {}
            void validate() const;
            Rate foreignRiskFreeRate;
            double exchangeRateVolatility;
            double correlation;
        };

        inline void QuantoOptionParameters::validate() const {

            VanillaOptionParameters::validate();

            QL_REQUIRE(foreignRiskFreeRate != Null<double>(),
                       "null risk free rate given");
            QL_REQUIRE(exchangeRateVolatility != Null<double>(),
                       "null exchange rate volatility given");
            QL_REQUIRE(exchangeRateVolatility >= 0.0,
                       "negative exchange rate volatility given");
            QL_REQUIRE(correlation != Null<double>(),
                       "null correlation given");
        }

        //! %results from quanto option calculation
        class QuantoOptionResults : public OptionValue {
        public:
            QuantoOptionResults(): delta(Null<double>()),
                gamma(Null<double>()), theta(Null<double>()),
                vega(Null<double>()), rho(Null<double>()),
                dividendRho(Null<double>()),
                vega2(Null<double>()), rho2(Null<double>()),
                lambda(Null<double>()) {}
            double delta, gamma, theta, vega, rho, dividendRho;
            double vega2, rho2, lambda;
        };

    
    }

}

#endif

