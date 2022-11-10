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

#ifndef QL_PATCH_SOLARIS

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/event.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/experimental/credit/gaussianlhplossmodel.hpp>
#include <ql/experimental/credit/midpointcdoengine.hpp>

using namespace std;

namespace QuantLib {

    SyntheticCDO::SyntheticCDO(const ext::shared_ptr<Basket>& basket,
                               Protection::Side side,
                               const Schedule& schedule,
                               Rate upfrontRate,
                               Rate runningRate,
                               const DayCounter& dayCounter,
                               BusinessDayConvention paymentConvention,
                               boost::optional<Real> notional)
    : basket_(basket), side_(side), upfrontRate_(upfrontRate), runningRate_(runningRate),
      leverageFactor_(notional ? notional.get() / basket->trancheNotional() : Real(1.)), // NOLINT(readability-implicit-bool-conversion)
      dayCounter_(dayCounter), paymentConvention_(paymentConvention) {
        QL_REQUIRE(!basket->names().empty(), "basket is empty");
        // Basket inception must lie before contract protection start.
        QL_REQUIRE(basket->refDate() <= schedule.startDate(),
        //using the start date of the schedule might be wrong, think of the 
        //  CDS rule
            "Basket did not exist before contract start.");

        // Notice the notional is that of the basket at basket inception, some 
        //   names might have defaulted in between
        normalizedLeg_ = FixedRateLeg(schedule)
            .withNotionals(basket_->trancheNotional() * leverageFactor_)
            .withCouponRates(runningRate, dayCounter)
            .withPaymentAdjustment(paymentConvention);

        // Date today = Settings::instance().evaluationDate();
        
        // register with probabilities if the corresponding issuer is, baskets
        //   are not registered with the DTS
        for (Size i = 0; i < basket->names().size(); i++) {
            /* This turns out to be a problem: depends on today but I am not 
            modifying the registrations, if we go back in time in the 
            calculations this would left me unregistered to some. Not impossible
            to de-register and register when updating but i am dropping it.

            if(!basket->pool()->get(basket->names()[i]).
                defaultedBetween(schedule.dates()[0], today,
                                     basket->pool()->defaultKeys()[i]))
            */
            // registers with the associated curve (issuer and event type)
            // \todo make it possible to access them by name instead of index
            registerWith(basket->pool()->get(basket->names()[i]).
                defaultProbability(basket->pool()->defaultKeys()[i]));
            /* \todo Issuers should be observables/obsrvr and they would in turn
            regiter with the DTS; only we might get updates from curves we do
            not use.
            */
        }
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
        return protectionValue_;
    }

    Rate SyntheticCDO::fairPremium () const {
        calculate();
        QL_REQUIRE(premiumValue_ != 0, "Attempted divide by zero while calculating syntheticCDO premium.");
        return runningRate_
            * (protectionValue_ - upfrontPremiumValue_) / premiumValue_;
    }

    Rate SyntheticCDO::fairUpfrontPremium () const {
        calculate();
        return (protectionValue_ - premiumValue_) / remainingNotional_;
    }

    std::vector<Real> SyntheticCDO::expectedTrancheLoss() const {
        calculate();
        return expectedTrancheLoss_;
    }

    Size SyntheticCDO::error () const {
        calculate();
        return error_;
    }

    bool SyntheticCDO::isExpired () const {
        // FIXME: it could have also expired (knocked out) because theres
        //   no remaining tranche notional.
        return detail::simple_event(normalizedLeg_.back()->date())
               .hasOccurred();
    }

    Real SyntheticCDO::remainingNotional() const {
        calculate();
        return remainingNotional_;
    }

    void SyntheticCDO::setupArguments(PricingEngine::arguments* args) const {
        auto* arguments = dynamic_cast<SyntheticCDO::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type");
        arguments->basket = basket_;
        arguments->side = side_;
        arguments->normalizedLeg = normalizedLeg_;

        arguments->upfrontRate = upfrontRate_;
        arguments->runningRate = runningRate_;
        arguments->dayCounter = dayCounter_;
        arguments->paymentConvention = paymentConvention_;
        arguments->leverageFactor = leverageFactor_;
    }

    void SyntheticCDO::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);

        const auto* results = dynamic_cast<const SyntheticCDO::results*>(r);
        QL_REQUIRE(results != nullptr, "wrong result type");

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





    namespace {

        class ObjectiveFunction {
          public:
            ObjectiveFunction(Real target,
                              SimpleQuote& quote,
                              PricingEngine& engine,
                              const SyntheticCDO::results* results)
            : target_(target), quote_(quote),
              engine_(engine), results_(results) {}

            Real operator()(Real guess) const {
                quote_.setValue(guess);
                engine_.calculate();
                return results_->value - target_;
            }
          private:
            Real target_;
            SimpleQuote& quote_;
            PricingEngine& engine_;
            const SyntheticCDO::results* results_;
        };

    }

    // untested, not sure this is not messing up, once it comes out of this
    //   the basket model is different.....
    Real SyntheticCDO::implicitCorrelation(const std::vector<Real>& recoveries, 
        const Handle<YieldTermStructure>& discountCurve, 
        Real targetNPV,
        Real accuracy) const 
    {
        ext::shared_ptr<SimpleQuote> correl(new SimpleQuote(0.0));

        ext::shared_ptr<GaussianLHPLossModel> lhp(new 
            GaussianLHPLossModel(Handle<Quote>(correl), recoveries));

        // lock
        basket_->setLossModel(lhp);

        MidPointCDOEngine engineIC(discountCurve);
        setupArguments(engineIC.getArguments());
        const auto* results = dynamic_cast<const SyntheticCDO::results*>(engineIC.getResults());

        // aviod recal of the basket on engine updates through the quote
        basket_->recalculate();
        basket_->freeze();

        ObjectiveFunction f(targetNPV, *correl, engineIC, results);
        Rate guess = 0.001;
        //  Rate step = guess*0.1;

        // wrap/catch to be able to unfreeze the basket:
        Real solution = Brent().solve(f, accuracy, guess, QL_EPSILON, 1.-QL_EPSILON);
        basket_->unfreeze();
        return solution;
    }

}

#endif
