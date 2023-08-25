/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file analyticcomplexchooserengine.hpp
    \brief Analytic engine for complex chooser option
*/

#ifndef quantlib_analytic_complex_chooser_engine_hpp
#define quantlib_analytic_complex_chooser_engine_hpp

#include <ql/instruments/complexchooseroption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/pricingengines/blackscholescalculator.hpp>

namespace QuantLib {

    class AnalyticComplexChooserEngine : public ComplexChooserOption::engine {
      public:
        explicit AnalyticComplexChooserEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;

        Real strike(Option::Type optionType) const;
        Time choosingTime() const;
        Time putMaturity() const;
        Time callMaturity() const;
        Volatility volatility(Time t) const;

        Rate dividendYield(Time t) const;
        DiscountFactor dividendDiscount(Time t) const;

        Rate riskFreeRate(Time t) const;
        DiscountFactor riskFreeDiscount(Time t) const;

        BlackScholesCalculator bsCalculator(Real spot,
                                            Option::Type optionType) const;
        Real criticalValue() const;
    };

}


#endif
