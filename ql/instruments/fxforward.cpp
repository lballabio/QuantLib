/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Chirag Desai

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

    FxForward::FxForward(Real sourceNominal,
                         const Currency& sourceCurrency,
                         Real targetNominal,
                         const Currency& targetCurrency,
                         const Date& maturityDate,
                         bool paySourceCurrency)
    : sourceNominal_(sourceNominal), sourceCurrency_(sourceCurrency), targetNominal_(targetNominal),
      targetCurrency_(targetCurrency), maturityDate_(maturityDate),
      paySourceCurrency_(paySourceCurrency), fairForwardRate_(Null<Real>()),
      npvSourceCurrency_(Null<Real>()), npvTargetCurrency_(Null<Real>()) {
        QL_REQUIRE(!sourceCurrency.empty(), "source currency must not be empty");
        QL_REQUIRE(!targetCurrency.empty(), "target currency must not be empty");
        QL_REQUIRE(sourceCurrency != targetCurrency,
                   "source and target currencies must be different");
        QL_REQUIRE(sourceNominal > 0.0, "source nominal must be positive");
        QL_REQUIRE(targetNominal > 0.0, "target nominal must be positive");
    }

    FxForward::FxForward(Real sourceNominal,
                         const Currency& sourceCurrency,
                         const Currency& targetCurrency,
                         Real forwardRate,
                         const Date& maturityDate,
                         bool sellingSource)
    : sourceNominal_(sourceNominal), sourceCurrency_(sourceCurrency),
      targetNominal_(sourceNominal * forwardRate), targetCurrency_(targetCurrency),
      maturityDate_(maturityDate), paySourceCurrency_(sellingSource),
      fairForwardRate_(Null<Real>()), npvSourceCurrency_(Null<Real>()),
      npvTargetCurrency_(Null<Real>()) {
        QL_REQUIRE(!sourceCurrency.empty(), "source currency must not be empty");
        QL_REQUIRE(!targetCurrency.empty(), "target currency must not be empty");
        QL_REQUIRE(sourceCurrency != targetCurrency,
                   "source and target currencies must be different");
        QL_REQUIRE(sourceNominal > 0.0, "source nominal must be positive");
        QL_REQUIRE(forwardRate > 0.0, "forward rate must be positive");
    }

    bool FxForward::isExpired() const {
        return maturityDate_ < Settings::instance().evaluationDate();
    }

    void FxForward::setupArguments(PricingEngine::arguments* args) const {
        auto* arguments = dynamic_cast<FxForward::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        arguments->sourceNominal = sourceNominal_;
        arguments->sourceCurrency = sourceCurrency_;
        arguments->targetNominal = targetNominal_;
        arguments->targetCurrency = targetCurrency_;
        arguments->maturityDate = maturityDate_;
        arguments->paySourceCurrency = paySourceCurrency_;
    }

    void FxForward::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);

        const auto* results = dynamic_cast<const FxForward::results*>(r);
        QL_REQUIRE(results != nullptr, "wrong result type");

        fairForwardRate_ = results->fairForwardRate;
        npvSourceCurrency_ = results->npvSourceCurrency;
        npvTargetCurrency_ = results->npvTargetCurrency;
    }

    Real FxForward::fairForwardRate() const {
        calculate();
        QL_REQUIRE(fairForwardRate_ != Null<Real>(), "fair forward rate not available");
        return fairForwardRate_;
    }

    Real FxForward::npvSourceCurrency() const {
        calculate();
        QL_REQUIRE(npvSourceCurrency_ != Null<Real>(), "NPV in source currency not available");
        return npvSourceCurrency_;
    }

    Real FxForward::npvTargetCurrency() const {
        calculate();
        QL_REQUIRE(npvTargetCurrency_ != Null<Real>(), "NPV in target currency not available");
        return npvTargetCurrency_;
    }

    void FxForward::arguments::validate() const {
        QL_REQUIRE(sourceNominal != Null<Real>(), "source nominal not set");
        QL_REQUIRE(targetNominal != Null<Real>(), "target nominal not set");
        QL_REQUIRE(!sourceCurrency.empty(), "source currency not set");
        QL_REQUIRE(!targetCurrency.empty(), "target currency not set");
        QL_REQUIRE(maturityDate != Date(), "maturity date not set");
    }

    void FxForward::results::reset() {
        Instrument::results::reset();
        fairForwardRate = Null<Real>();
        npvSourceCurrency = Null<Real>();
        npvTargetCurrency = Null<Real>();
    }

}
