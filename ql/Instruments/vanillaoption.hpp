

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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
/*! \file vanillaoption.hpp
    \brief Vanilla (no discrete dividends, no barriers) option on a single asset

    \fullpath
    ql/Instruments/%vanillaoption.hpp
*/

// $Id$

#ifndef quantlib_vanilla_option_h
#define quantlib_vanilla_option_h

#include <ql/option.hpp>
#include <ql/termstructure.hpp>
#include <ql/solver1d.hpp>
#include <ql/PricingEngines/vanillaengines.hpp>

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
                          const Date& exerciseDate,
                          const RelinkableHandle<BlackVolTermStructure>& volTS,
                          const Handle<PricingEngine>& engine,
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
            // results
            mutable double delta_, gamma_, theta_, vega_, rho_, dividendRho_;
            // arguments
            Option::Type type_;
            RelinkableHandle<MarketElement> underlying_;
            double strike_;
            Date exerciseDate_;
            RelinkableHandle<TermStructure> riskFreeTS_, dividendTS_;
            RelinkableHandle<BlackVolTermStructure> volTS_;
          private:
            // helper class for implied volatility calculation
            class ImpliedVolHelper : public ObjectiveFunction {
              public:
                ImpliedVolHelper(const Handle<PricingEngine>& engine,
                                 double targetValue);
                double operator()(double x) const;
              private:
                Handle<PricingEngine> engine_;
                double targetValue_;
                PricingEngines::VanillaOptionArguments* arguments_;
                const OptionValue* results_;
            };
        };

    }


}


#endif

