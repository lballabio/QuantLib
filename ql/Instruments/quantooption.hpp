

/*
 Copyright (C) 2002 Ferdinando Ametrano

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
/*! \file quantooption.hpp
    \brief Quanto option on a single asset

    \fullpath
    ql/Instruments/%quantooption.hpp
*/

// $Id$

#ifndef quantlib_quanto_option_h
#define quantlib_quanto_option_h

#include <ql/option.hpp>
#include <ql/termstructure.hpp>
#include <ql/solver1d.hpp>
#include <ql/PricingEngines/quantoengines.hpp>

namespace QuantLib {

    namespace Instruments {

        //! Quanto option on a single asset
        class QuantoOption : public Option {
          public:
            QuantoOption(
                Option::Type type,
                const RelinkableHandle<MarketElement>& underlying,
                double strike,
                const RelinkableHandle<TermStructure>& dividendYield,
                const RelinkableHandle<TermStructure>& riskFreeRate,
                const Date& exerciseDate,
                const RelinkableHandle<MarketElement>& volatility,
                const RelinkableHandle<TermStructure>& foreignRiskFreeRate,
                const RelinkableHandle<MarketElement>& exchangeRateVolatility,
                const RelinkableHandle<MarketElement>& correlation,
                const Handle<PricingEngines::QuantoEngine>& engine,
                const std::string& isinCode = "",
                const std::string& description = "");
            //! \name greeks
            //@{
            double delta() const;
            double gamma() const;
            double theta() const;
            double vega() const;
            double rho() const;
            double dividendRho() const;
            double vega2() const;
            double rho2() const;
            double lambda() const;
            //@}
            /*! \warning Options with a gamma that changes sign have values
                that are <b>not</b> monotonic in the volatility, e.g binary
                options. In these cases impliedVolatility can fail and in
                any case it is almost meaningless.
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
            RelinkableHandle<TermStructure> foreignRiskFreeRate_;
            RelinkableHandle<MarketElement> exchangeRateVolatility_;
            RelinkableHandle<MarketElement> correlation_;
            // results
            mutable double delta_, gamma_, theta_, vega_, rho_, dividendRho_;
            mutable double vega2_, rho2_, lambda_;
            // helper class for implied volatility calculation
            class ImpliedVolHelper : public ObjectiveFunction {
              public:
                ImpliedVolHelper(const Handle<PricingEngine>& engine,
                                 double targetValue);
                double operator()(double x) const;
              private:
                Handle<PricingEngine> engine_;
                double targetValue_;
                PricingEngines::QuantoOptionParameters* parameters_;
                const OptionValue* results_;
            };
        };

    }


}


#endif

