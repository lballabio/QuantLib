
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

/*! \file plainoption.hpp
    \brief Plain (no dividends, no barriers) option on a single asset

    \fullpath
    ql/Instruments/%plainoption.hpp
*/

// $Id$

#ifndef quantlib_plain_option_h
#define quantlib_plain_option_h

#include "ql/option.hpp"
#include "ql/marketelement.hpp"
#include "ql/termstructure.hpp"
#include "ql/solver1d.hpp"

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
            double impliedVolatility(double targetValue, 
                                     double accuracy = 1.0e-4,
                                     size_t maxEvaluations = 100,
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
            Rate dividendYield, riskFreeRate;
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

