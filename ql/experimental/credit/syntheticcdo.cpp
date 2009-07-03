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

#include <ql/experimental/credit/syntheticcdo.hpp>
#include <ql/event.hpp>

using namespace std;

namespace QuantLib {

    SyntheticCDO::SyntheticCDO (const boost::shared_ptr<Basket> basket,
                                Protection::Side side,
                                const Schedule& schedule,
                                Rate upfrontRate,
                                Rate runningRate,
                                const DayCounter& dayCounter,
                                BusinessDayConvention paymentConvention,
                                const Handle<YieldTermStructure>& yieldTS)
        : basket_(basket),
          side_(side),
          schedule_(schedule),
          upfrontRate_(upfrontRate),
          runningRate_(runningRate),
          dayCounter_(dayCounter),
          paymentConvention_(paymentConvention),
          yieldTS_(yieldTS) {
        QL_REQUIRE (basket->names().size() > 0, "basket is empty");

        registerWith (yieldTS_);
        const boost::shared_ptr<Pool> pool = basket->pool();

        Date today = Settings::instance().evaluationDate();
        // register with probabilities if the corresponding issuer is
        // alive under this name contractual conditions
        for (Size i = 0; i < basket->names().size(); i++) {
            if(!pool->get(basket->names()[i]).
                defaultedBetween(schedule.dates()[0],
                                 today,
                                 basket->defaultKeys()[i]))
            registerWith(pool->get(basket->names()[i]).
                defaultProbability(basket->defaultKeys()[i]));
            // To do: the basket could do this last line on its own without so much
            //    travelling, update its interface? some RR models depend on the
            //    probabilities and they will be registered with them. Strictly
            //    speaking the basket does not need directly to be registered
            //    with the probs.
            /*
                we used to be registering with Issuers which are not observables. However
                this might be what we want: Issuer registration might give us registration
                with probabilities and with creditEvents at the same time.
            */
        }
        // register with recoveries:
        registerWith(basket_);
    }

    Rate SyntheticCDO::premiumValue () const {
        calculate();
        return premiumValue_;
    }

    Rate SyntheticCDO::protectionValue () const {
        calculate();
        return protectionValue_;
    }

    Real SyntheticCDO::premiumLegNPV() const {
        calculate();
        if(side_ == Protection::Buyer) return premiumValue_;
        return -premiumValue_;
    }

    Real SyntheticCDO::protectionLegNPV() const {
        calculate();
        if(side_ == Protection::Buyer) return -protectionValue_;
        return premiumValue_;
    }

    Rate SyntheticCDO::fairPremium () const {
        calculate();
        return runningRate_
            * (protectionValue_ - upfrontPremiumValue_) / premiumValue_;
    }

    Rate SyntheticCDO::fairUpfrontPremium () const {
        calculate();
        return (protectionValue_ - premiumValue_) / remainingNotional_;
    }

    vector<Real> SyntheticCDO::expectedTrancheLoss() const {
        calculate();
        return expectedTrancheLoss_;
    }

    Size SyntheticCDO::error () const {
        calculate();
        return error_;
    }

    bool SyntheticCDO::isExpired () const {
        return detail::simple_event(schedule_.dates().back())
               .hasOccurred(yieldTS_->referenceDate());
    }

    Real SyntheticCDO::remainingNotional() const {
        calculate();
        return remainingNotional_;
    }

    void SyntheticCDO::setupArguments(PricingEngine::arguments* args) const {
        SyntheticCDO::arguments* arguments
            = dynamic_cast<SyntheticCDO::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");
        arguments->basket = basket_;
        arguments->side = side_;
        arguments->schedule = schedule_;
        arguments->upfrontRate = upfrontRate_;
        arguments->runningRate = runningRate_;
        arguments->dayCounter = dayCounter_;
        arguments->paymentConvention = paymentConvention_;
        arguments->yieldTS = yieldTS_;
    }

    void SyntheticCDO::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);

        const SyntheticCDO::results* results
            = dynamic_cast<const SyntheticCDO::results*>(r);
        QL_REQUIRE(results != 0, "wrong result type");

        premiumValue_ = results->premiumValue;
        protectionValue_ = results->protectionValue;
        upfrontPremiumValue_ = results->upfrontPremiumValue;
        remainingNotional_ = results->remainingNotional;
        error_ = results->error;
        expectedTrancheLoss_ = results->expectedTrancheLoss;
    }

    void SyntheticCDO::setupExpired() const {
        Instrument::setupExpired();
        premiumValue_ = 0.0;
        protectionValue_ = 0.0;
        upfrontPremiumValue_ = 0.0;
        remainingNotional_ = 1.0;
        expectedTrancheLoss_.clear();
    }

    void SyntheticCDO::arguments::validate() const {
        QL_REQUIRE(side != Protection::Side(-1), "side not set");
        QL_REQUIRE(basket && !basket->names().empty(), "no basket given");
        QL_REQUIRE(runningRate != Null<Real>(), "no premium rate given");
        QL_REQUIRE(upfrontRate != Null<Real>(), "no upfront rate given");
        QL_REQUIRE(!dayCounter.empty(), "no day counter given");
        QL_REQUIRE(!yieldTS.empty(), "no discount curve given");
    }

    void SyntheticCDO::results::reset() {
        Instrument::results::reset();
        premiumValue = Null<Real>();
        protectionValue = Null<Real>();
        upfrontPremiumValue = Null<Real>();
        remainingNotional = Null<Real>();
        error = 0;
        expectedTrancheLoss.clear();
    }

}
