

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
/*! \file plainoption.hpp
    \brief Plain (no dividends, no barriers) option on a single asset

    \fullpath
    ql/Instruments/%plainoption.hpp
*/

// $Id$

#ifndef quantlib_plain_option_h
#define quantlib_plain_option_h

#include <ql/option.hpp>
#include <ql/termstructure.hpp>
#include <ql/solver1d.hpp>

namespace QuantLib {

    namespace Instruments {

    class PlainOptionParameters;

        //! Plain (no dividends, no barriers) option on a single asset
        class PlainOption : public Option {
          public:
            PlainOption(Option::Type type,
                        const RelinkableHandle<MarketElement>& underlying,
                        double strike,
                        const RelinkableHandle<TermStructure>& dividendYield,
                        const RelinkableHandle<TermStructure>& riskFreeRate,
                        const Date& exerciseDate,
                        const RelinkableHandle<MarketElement>& volatility,
                        const Handle<OptionPricingEngine>& engine,
                        const std::string& isinCode = "",
                        const std::string& description = "");
            ~PlainOption();
            //! \name greeks
            //@{
            double delta() const;
            double gamma() const;
            double theta() const;
            double vega() const;
            double rho() const;
            double dividendRho() const;
            //@}
            /*! \warning Options with a gamma that changes sign have values
                that are <b>not</b> monotonic in the volatility, e.g binary
                options. In these cases impliedVolatility can fail and in
                any case is meaningless.
                Another possible source of failure is to have a
                targetValue that is not attainable with any volatility, e.g.
                a targetValue lower than the intrinsic value in the case of
                American options. */
            double impliedVolatility(double targetValue,
                                     double accuracy = 1.0e-4,
                                     Size maxEvaluations = 100,
                                     double minVol = 1.0e-4,
                                     double maxVol = 4.0) const;
          protected:
            void setupEngine() const;
            void performCalculations() const;
          private:
            // parameters
            Option::Type type_;
            RelinkableHandle<MarketElement> underlying_;
            double strike_;
            RelinkableHandle<TermStructure> dividendYield_, riskFreeRate_;
            Date exerciseDate_;
            RelinkableHandle<MarketElement> volatility_;
            // results
            mutable double delta_, gamma_, theta_, vega_, rho_, dividendRho_;
            // helper class for implied volatility calculation
            class ImpliedVolHelper : public ObjectiveFunction {
              public:
                ImpliedVolHelper(const Handle<OptionPricingEngine>& engine,
                                 double targetValue);
                double operator()(double x) const;
              private:
                Handle<OptionPricingEngine> engine_;
                double targetValue_;
                PlainOptionParameters* parameters_;
                const OptionValue* results_;
            };
        };

        //! parameters for plain option calculation
        class PlainOptionParameters : public virtual Arguments {
          public:
            PlainOptionParameters() : type(Option::Type(-1)),
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

        //! %results from plain option calculation
        class PlainOptionResults : public OptionValue, public OptionGreeks {};

    }

    namespace Pricers {

        //! base class for plain option pricing engines
        /*! Derived engines only need to implement the <tt>calculate()</tt>
            method
        */
        class PlainOptionEngine : public OptionPricingEngine {
          public:
            Arguments* parameters();
            void validateParameters() const;
            const Results* results() const;
          protected:
            Instruments::PlainOptionParameters parameters_;
            mutable Instruments::PlainOptionResults results_;
        };

    }

}


#endif

