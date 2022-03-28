/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

#include <ql/experimental/credit/nthtodefault.hpp>
#include <ql/experimental/credit/lossdistribution.hpp>
#include <ql/instruments/claim.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/event.hpp>
#include <ql/experimental/credit/basket.hpp>

namespace QuantLib {


    NthToDefault::NthToDefault(
        const ext::shared_ptr<Basket>& basket,
        Size n,
        Protection::Side side,
        const Schedule& premiumSchedule,
        Rate upfrontRate,
        Rate premiumRate,
        const DayCounter& dayCounter,
        Real nominal,
        bool settlePremiumAccrual
        )
    : basket_(basket), n_(n),
      side_(side), nominal_(nominal),
      premiumSchedule_(premiumSchedule), premiumRate_(premiumRate), 
      upfrontRate_(upfrontRate), 
      dayCounter_(dayCounter), settlePremiumAccrual_(settlePremiumAccrual)
    {
        QL_REQUIRE(n_ <= basket_->size(), 
                   "NTD order provided is larger than the basket size.");

        // Basket inception must lie before contract protection start.
        QL_REQUIRE(basket->refDate() <= premiumSchedule.startDate(),
            //using the start date of the schedule might be wrong, think of the CDS rule
            "Basket did not exist before contract start.");

        premiumLeg_ = FixedRateLeg(premiumSchedule)
            .withNotionals(nominal)
            .withCouponRates(premiumRate, dayCounter)
            .withPaymentAdjustment(Unadjusted);

        registerWith(basket_);
    }


// SOME OF THESE ARE INLINES---------------------------------
    Size NthToDefault::basketSize() const { return basket_->size(); }

    bool NthToDefault::isExpired() const {
        return detail::simple_event(premiumLeg_.back()->date()).hasOccurred();
    }

    Rate NthToDefault::fairPremium() const {
        calculate();
        QL_REQUIRE(fairPremium_ != Null<Rate>(),
                   "fair premium not available");
        return fairPremium_;
    }

    Real NthToDefault::premiumLegNPV() const {
        calculate();
        QL_REQUIRE(premiumValue_ != Null<Rate>(),
                   "premium leg not available");
        QL_REQUIRE(upfrontPremiumValue_ != Null<Rate>(),
                   "upfront value not available");
        return premiumValue_ + upfrontPremiumValue_;
    }

    Real NthToDefault::protectionLegNPV() const {
        calculate();
        QL_REQUIRE(protectionValue_ != Null<Rate>(),
                   "protection leg not available");
        return protectionValue_;
    }

    Real NthToDefault::errorEstimate() const {
        calculate();
        QL_REQUIRE(errorEstimate_ != Null<Rate>(),
                   "error estimate not available");
        return errorEstimate_;

    }

    void NthToDefault::setupExpired() const {
        Instrument::setupExpired();

        premiumValue_ = 0.0;
        protectionValue_ = 0.0;
        upfrontPremiumValue_ = 0.0;
        fairPremium_ = 0.0;
        errorEstimate_ = 0.0;
    }

    void NthToDefault::setupArguments(PricingEngine::arguments* args) const {
        auto* arguments = dynamic_cast<NthToDefault::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");
        arguments->basket = basket_;
        arguments->side = side_;
        arguments->premiumLeg = premiumLeg_;
        arguments->ntdOrder = n_;
        arguments->settlePremiumAccrual = settlePremiumAccrual_;
        arguments->notional = nominal_;
        arguments->premiumRate = premiumRate_;
        arguments->upfrontRate = upfrontRate_;
    }

    void NthToDefault::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);

        const auto* results = dynamic_cast<const NthToDefault::results*>(r);
        QL_REQUIRE(results != 0, "wrong result type");

        premiumValue_ = results->premiumValue;
        protectionValue_ = results->protectionValue;
        upfrontPremiumValue_ = results->upfrontPremiumValue;
        fairPremium_ = results->fairPremium;
        errorEstimate_ = results->errorEstimate;
    }

    void NthToDefault::results::reset() {
        Instrument::results::reset();
        premiumValue = Null<Real>();
        protectionValue = Null<Real>();
        upfrontPremiumValue = Null<Real>();
        fairPremium = Null<Real>();
        errorEstimate = Null<Real>();
        additionalResults.clear();
    }

    void NthToDefault::arguments::validate() const {
        QL_REQUIRE(basket && !basket->names().empty(), "no basket given");
        QL_REQUIRE(side != Protection::Side(-1), "side not set");
        QL_REQUIRE(premiumRate != Null<Real>(), "no premium rate given");
        QL_REQUIRE(upfrontRate != Null<Real>(), "no upfront rate given");
        QL_REQUIRE(notional != Null<Real>(), "no notional given");
        QL_REQUIRE(ntdOrder != Null<Size>(), "no NTD order given");
    }

}

