
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file vanillaoption.hpp
    \brief Vanilla option on a single asset
*/

#ifndef quantlib_vanilla_option_h
#define quantlib_vanilla_option_h

#include <ql/option.hpp>
#include <ql/termstructure.hpp>
#include <ql/voltermstructure.hpp>
#include <ql/exercise.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

    namespace Instruments {

    //! Vanilla option (no discrete dividends, no barriers) on a single asset
        class VanillaOption : public Option {
          public:
            VanillaOption(Option::Type type,
                          const RelinkableHandle<MarketElement>& underlying,
                          double strike,
                          const RelinkableHandle<TermStructure>& dividendTS,
                          const RelinkableHandle<TermStructure>& riskFreeTS,
                          const Exercise& exercise,
                          const RelinkableHandle<BlackVolTermStructure>& volTS,
                          const Handle<PricingEngine>& engine =
                                                   Handle<PricingEngine>(),
                          const std::string& isinCode = "",
                          const std::string& description = "");
            //! \name Instrument interface
            //@{
            class arguments;
            class results;
            bool isExpired() const;
            //@}
            //! \name greeks
            //@{
            double delta() const;
            double gamma() const;
            double theta() const;
            double vega() const;
            double rho() const;
            double dividendRho() const;
            double strikeSensitivity() const;
            //@}
            /*! \warning Options with a gamma that changes sign have values
                that are <b>not</b> monotonic in the volatility, e.g binary
                options. In these cases impliedVolatility can fail and in
                any case it is almost meaningless.
                Another possible source of failure is to have a
                targetValue that is not attainable with any volatility, e.g.
                a targetValue lower than the intrinsic value in the case of
                American options. */
            double impliedVolatility(double price,
                                     double accuracy = 1.0e-4,
                                     Size maxEvaluations = 100,
                                     double minVol = 1.0e-4,
                                     double maxVol = 4.0) const;
            void setupArguments(Arguments*) const;
          protected:
            void setupExpired() const;
            void performCalculations() const;
            // results
            mutable double delta_, gamma_, theta_, 
                           vega_, rho_, dividendRho_, strikeSensitivity_;
            // arguments
            Option::Type type_;
            RelinkableHandle<MarketElement> underlying_;
            double strike_;
            Exercise exercise_;
            RelinkableHandle<TermStructure> riskFreeTS_, dividendTS_;
            RelinkableHandle<BlackVolTermStructure> volTS_;
          private:
            // helper class for implied volatility calculation
            class ImpliedVolHelper {
              public:
                ImpliedVolHelper(const Handle<PricingEngine>& engine,
                                 double targetValue);
                double operator()(double x) const;
              private:
                Handle<PricingEngine> engine_;
                double targetValue_;
                Handle<SimpleMarketElement> vol_;
                const Value* results_;
            };
        };

        //! arguments for vanilla option calculation
        class VanillaOption::arguments : public virtual Arguments {
          public:
            arguments() : underlying(Null<double>()),
                          maturity(Null<Time>()) {}
            void validate() const;
            Handle<Payoff> payoff;
            double underlying;
            RelinkableHandle<TermStructure> riskFreeTS, dividendTS;
            RelinkableHandle<BlackVolTermStructure> volTS;
            // we need to calculate option at times that are not
            // generated by real dates. So we need to set t, not a Date
            // that's why we cannot use Exercise below
            Exercise::Type exerciseType;
            std::vector<Time> stoppingTimes;
            Time maturity;
        };

        //! %results from vanilla option calculation
        class VanillaOption::results : public Value, public Greeks {
          public:
            void reset() {
                Value::reset();
                Greeks::reset();
            }
        };

    }

}


#endif

