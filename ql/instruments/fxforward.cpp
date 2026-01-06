/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/instruments/fxforward.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    FxForward::FxForward(Real nominal1,
                         const Currency& currency1,
                         Real nominal2,
                         const Currency& currency2,
                         const Date& maturityDate,
                         bool payCurrency1)
    : nominal1_(nominal1), currency1_(currency1), nominal2_(nominal2), currency2_(currency2),
      maturityDate_(maturityDate), payCurrency1_(payCurrency1), fairForwardRate_(Null<Real>()),
      npvCurrency1_(Null<Real>()), npvCurrency2_(Null<Real>()) {
        QL_REQUIRE(!currency1.empty(), "currency1 must not be empty");
        QL_REQUIRE(!currency2.empty(), "currency2 must not be empty");
        QL_REQUIRE(currency1 != currency2, "currency1 and currency2 must be different");
        QL_REQUIRE(nominal1 > 0.0, "nominal1 must be positive");
        QL_REQUIRE(nominal2 > 0.0, "nominal2 must be positive");
    }

    FxForward::FxForward(Real nominal,
                         const Currency& sourceCurrency,
                         const Currency& targetCurrency,
                         Real forwardRate,
                         const Date& maturityDate,
                         bool sellingSource)
    : nominal1_(nominal), currency1_(sourceCurrency), nominal2_(nominal * forwardRate),
      currency2_(targetCurrency), maturityDate_(maturityDate), payCurrency1_(sellingSource),
      fairForwardRate_(Null<Real>()), npvCurrency1_(Null<Real>()), npvCurrency2_(Null<Real>()) {
        QL_REQUIRE(!sourceCurrency.empty(), "source currency must not be empty");
        QL_REQUIRE(!targetCurrency.empty(), "target currency must not be empty");
        QL_REQUIRE(sourceCurrency != targetCurrency,
                   "source and target currencies must be different");
        QL_REQUIRE(nominal > 0.0, "nominal must be positive");
        QL_REQUIRE(forwardRate > 0.0, "forward rate must be positive");
    }

    bool FxForward::isExpired() const {
        return maturityDate_ < Settings::instance().evaluationDate();
    }

    void FxForward::setupArguments(PricingEngine::arguments* args) const {
        auto* arguments = dynamic_cast<FxForward::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        arguments->nominal1 = nominal1_;
        arguments->currency1 = currency1_;
        arguments->nominal2 = nominal2_;
        arguments->currency2 = currency2_;
        arguments->maturityDate = maturityDate_;
        arguments->payCurrency1 = payCurrency1_;
    }

    void FxForward::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);

        const auto* results = dynamic_cast<const FxForward::results*>(r);
        QL_REQUIRE(results != nullptr, "wrong result type");

        fairForwardRate_ = results->fairForwardRate;
        npvCurrency1_ = results->npvCurrency1;
        npvCurrency2_ = results->npvCurrency2;
    }

    Real FxForward::fairForwardRate() const {
        calculate();
        QL_REQUIRE(fairForwardRate_ != Null<Real>(), "fair forward rate not available");
        return fairForwardRate_;
    }

    Real FxForward::npvCurrency1() const {
        calculate();
        QL_REQUIRE(npvCurrency1_ != Null<Real>(), "NPV in currency1 not available");
        return npvCurrency1_;
    }

    Real FxForward::npvCurrency2() const {
        calculate();
        QL_REQUIRE(npvCurrency2_ != Null<Real>(), "NPV in currency2 not available");
        return npvCurrency2_;
    }

    void FxForward::arguments::validate() const {
        QL_REQUIRE(nominal1 != Null<Real>(), "nominal1 not set");
        QL_REQUIRE(nominal2 != Null<Real>(), "nominal2 not set");
        QL_REQUIRE(!currency1.empty(), "currency1 not set");
        QL_REQUIRE(!currency2.empty(), "currency2 not set");
        QL_REQUIRE(maturityDate != Date(), "maturity date not set");
    }

    void FxForward::results::reset() {
        Instrument::results::reset();
        fairForwardRate = Null<Real>();
        npvCurrency1 = Null<Real>();
        npvCurrency2 = Null<Real>();
    }

}
