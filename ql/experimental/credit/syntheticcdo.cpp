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

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/experimental/credit/syntheticcdo.hpp>
#include <ql/event.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/experimental/credit/gaussianlhplossmodel.hpp>
#include <ql/experimental/credit/midpointcdoengine.hpp>

using namespace std;

namespace QuantLib {

    SyntheticCDO::SyntheticCDO(const boost::shared_ptr<Basket>& basket,
                               Protection::Side side,
                               const Schedule& schedule,
                               Rate upfrontRate,
                               Rate runningRate,
                               const DayCounter& dayCounter,
                               BusinessDayConvention paymentConvention,
                               boost::optional<Real> notional)
    : basket_(basket),
      side_(side),
      upfrontRate_(upfrontRate),
      runningRate_(runningRate),
      dayCounter_(dayCounter),
      paymentConvention_(paymentConvention),
      leverageFactor_(notional ? notional.get() / basket->trancheNotional() 
        : 1.) 
    {
        QL_REQUIRE (basket->names().size() > 0, "basket is empty");
     //PP   registerWith (yieldTS_);
/////////// NEEDS TO CHECK THAT THE SCHEDULE FIRST DATE LIES AFTER THE BASKET REFERENCE DATE!!!, I.E. OK FORWARD START BUT DO NOT TAKE DATES PRIOR TO THE BSKT EXISTANCE.....review the basket have a ref date at all?
        // Basket inception must lie before contract protection start.
        QL_REQUIRE(basket->refDate() <= schedule.startDate(),
            //using the start date of the schedule might be wrong, think of the CDS rule
            "Basket did not exist before contract start.");

        // Notice the notional is that of the basket at basket inception, some names might have defaulted in between
        normalizedLeg_ = FixedRateLeg(schedule)
            .withNotionals(basket_->trancheNotional() * leverageFactor_)// notional normalized to the live notional in the negine
            /* This normalized nottional business is a waste. Say I have two identical tranches on a bskt, one of them is leveraged..... I need to create a different basket just for it....!! Maybe I can pass a notional which I can compare to the original basket tranche notional and work out a leverage factor which to pass to the pricing engine.

            */
            .withCouponRates(runningRate, dayCounter)
            .withPaymentAdjustment(paymentConvention);

       //////////// const boost::shared_ptr<Pool> pool = basket->pool();

        Date today = Settings::instance().evaluationDate();
        // register with probabilities if the corresponding issuer is // IS IT NOT ENOUGH NOW TO REGISTER WITH THE BSKT?????????????????????????????????????
        // alive under this name contractual conditions
        for (Size i = 0; i < basket->names().size(); i++) {
            if(!basket->pool()->get(basket->names()[i]).
                defaultedBetween(schedule.dates()[0],
                                 today,// << problem, this depends on today but I am not modifying the registrations, if we go back in time in the calculations this would left me unregistered to some 
                                 basket->pool()->defaultKeys()[i]))
                                 // registers with the associated curve (issuer and event type)
            registerWith(basket->pool()->get(basket->names()[i]).
                defaultProbability(basket->pool()->defaultKeys()[i]));// SHOULD BE ACCESSING THEM BY NAME, not index
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

    Disposable<vector<Real> > SyntheticCDO::expectedTrancheLoss() const {
        calculate();
        return expectedTrancheLoss_;
    }

    Size SyntheticCDO::error () const {
        calculate();
        return error_;
    }

    bool SyntheticCDO::isExpired () const {
        // to do: it could have also expired (knocked out) because theres  <<<<<<<<<<<<<<<<<<<<<<<<<
        //   no remaining tranche notional.
        return detail::simple_event(normalizedLeg_.back()->date())
               .hasOccurred();
    }

    Real SyntheticCDO::remainingNotional() const {
        calculate();/// calculate?????  calculate the basket is enough 
        return remainingNotional_;
    }

    void SyntheticCDO::setupArguments(PricingEngine::arguments* args) const {
        SyntheticCDO::arguments* arguments
            = dynamic_cast<SyntheticCDO::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");
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

    Real SyntheticCDO::implicitCorrelation(const std::vector<Real>& recoveries, 
        const Handle<YieldTermStructure>& discountCurve, 
        Real targetNPV,
        Real accuracy) const 
    {
            // SHOULD SAVE AND RESTORE THE (POSSIBLE) ENGINE IN THE CDO..........AND FREEZE IT!! OTHERWISE WILL RECALC
        boost::shared_ptr<SimpleQuote> correl(new SimpleQuote(0.0));

        boost::shared_ptr<GaussianLHPLossModel> lhp(new 
            GaussianLHPLossModel(Handle<Quote>(correl), recoveries));

        // lock
        basket_->setLossModel(lhp);// intialization of the LHP model takes place here and since we do not reasign the basket theres no need to do it again in the ObjectiveFunction for every change of the correl quote.
        MidPointCDOEngine engineIC(discountCurve);
        setupArguments(engineIC.getArguments());
        const SyntheticCDO::results* results = 
            dynamic_cast<const SyntheticCDO::results*>(engineIC.getResults());

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
