
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

/*! \file oneassetoption.hpp
    \brief Option on a single asset
*/

#ifndef quantlib_oneasset_option_h
#define quantlib_oneasset_option_h

#include <ql/option.hpp>
#include <ql/termstructure.hpp>
#include <ql/voltermstructure.hpp>

namespace QuantLib {

    //! Base class for options on a single asset
    class OneAssetOption : public Option {
      public:
        OneAssetOption(const RelinkableHandle<Quote>& underlying,
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
        //@}
        /*! \warning Options with a gamma that changes sign have
                     values that are <b>not</b> monotonic in the
                     volatility, e.g binary options. In these cases
                     impliedVolatility can fail and in any case it is
                     almost meaningless.  Another possible source of
                     failure is to have a targetValue that is not
                     attainable with any volatility, e.g.  a
                     targetValue lower than the intrinsic value in the
                     case of American options. */
        double impliedVolatility(double price,
                                 double accuracy = 1.0e-4,
                                 Size maxEvaluations = 100,
                                 double minVol = QL_MIN_VOLATILITY,
                                 double maxVol = QL_MAX_VOLATILITY) const;
        void setupArguments(Arguments*) const;
      protected:
        void setupExpired() const;
        void performCalculations() const;
        // results
        mutable double delta_, gamma_, theta_,
                       vega_, rho_, dividendRho_;
        // arguments
        RelinkableHandle<Quote> underlying_;
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
            Handle<SimpleQuote> vol_;
            const Value* results_;
        };
    };

    //! arguments for single asset option calculation
    class OneAssetOption::arguments : public Option::arguments {
      public:
        arguments() : underlying(Null<double>()) {}
        void validate() const;
        double underlying;
        RelinkableHandle<TermStructure> riskFreeTS, dividendTS;
        RelinkableHandle<BlackVolTermStructure> volTS;
    };

    //! %results from single asset option calculation
    class OneAssetOption::results : public Value, public Greeks {
      public:
        void reset() {
            Value::reset();
            Greeks::reset();
        }
    };

}


#endif

