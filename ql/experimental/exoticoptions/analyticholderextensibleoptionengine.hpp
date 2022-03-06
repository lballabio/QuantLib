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

/*! \file analyticholderextensibleoptionengine.hpp
    \brief Analytic engine for holder-extensible options
*/

#ifndef quantlib_analytic_holder_extensible_option_engine_hpp
#define quantlib_analytic_holder_extensible_option_engine_hpp

#include <ql/experimental/exoticoptions/holderextensibleoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/pricingengines/blackscholescalculator.hpp>

namespace QuantLib {

    class AnalyticHolderExtensibleOptionEngine
        : public HolderExtensibleOption::engine {
      public:
        explicit AnalyticHolderExtensibleOptionEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Real strike() const;
        Time firstExpiryTime() const;
        Time secondExpiryTime() const;
        Volatility volatility() const;
        Rate riskFreeRate() const;
        Rate dividendYield() const;
        DiscountFactor dividendDiscount(Time t) const;
        DiscountFactor riskFreeDiscount(Time t) const;
        Real I1Call() const;
        Real I2Call() const;
        Real I1Put() const;
        Real I2Put() const;
        BlackScholesCalculator bsCalculator(Real spot, Option::Type optionType) const;
        Real M2(Real a, Real b, Real c, Real d, Real rho) const;
        Real N2(Real a, Real b) const;
        Real y1(Option::Type) const;
        Real y2(Option::Type) const;
        Real z1() const;
        Real z2() const;
    };

}


#endif


#ifndef id_52e667937b8564e7db0878527f2a45dc
#define id_52e667937b8564e7db0878527f2a45dc
inline bool test_52e667937b8564e7db0878527f2a45dc(int* i) { return i != 0; }
#endif
