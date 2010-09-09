/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Ferdinando Ametrano

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

#include <ql/instruments/bonds/btp.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/pricingengines/bond/bondfunctions.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    BTP::BTP(const Date& maturityDate,
             Rate fixedRate,
             Real redemption,
             const Date& startDate,
             const Date& issueDate)
    : FixedRateBond(3, 100.0,
                    Schedule(startDate,
                             maturityDate, 6*Months,
                             NullCalendar(), Unadjusted, Unadjusted,
                             DateGeneration::Backward, true),
                    std::vector<Rate>(1, fixedRate),
                    ActualActual(ActualActual::ISMA),
                    ModifiedFollowing, redemption, issueDate, TARGET()) {}

    Rate BTP::yield(Real cleanPrice,
                    Date settlementDate,
                    Real accuracy,
                    Size maxEvaluations) const {
        return Bond::yield(cleanPrice, ActualActual(ActualActual::ISMA),
                           Compounded, Annual,
                           settlementDate, accuracy, maxEvaluations);
    }


    RendistatoBasket::RendistatoBasket(
            const std::vector<boost::shared_ptr<BTP> >& btps,
            const std::vector<Real>& outstandings,
            const std::vector<Handle<Quote> >& cleanPriceQuotes)
    : btps_(btps), outstandings_(outstandings), quotes_(cleanPriceQuotes) {

        QL_REQUIRE(!btps_.empty(), "empty RendistatoCalculator Basket");
        Size k = btps_.size();

        QL_REQUIRE(outstandings_.size()==k,
                   "mismatch between number of BTPs (" << k <<
                   ") and number of outstandings (" <<
                   outstandings_.size() << ")");
        QL_REQUIRE(quotes_.size()==k,
                   "mismatch between number of BTPs (" << k <<
                   ") and number of clean prices quotes (" <<
                   quotes_.size() << ")");

        // require non-negative outstanding
        for (Size i=0; i<k; ++i) {
            QL_REQUIRE(outstandings[i]>=0,
                       "negative outstanding for " << io::ordinal(i) <<
                       " bond, maturity " << btps[i]->maturityDate());
            // add check for prices ??
        }

        // TODO: filter out expired bonds, zero outstanding bond, etc

        QL_REQUIRE(!btps_.empty(), "invalid bonds only in RendistatoCalculator Basket");
        n_ = btps_.size();

        outstanding_ = 0.0;
        for (Size i=0; i<n_; ++i)
            outstanding_ += outstandings[i];

        weights_.resize(n_);
        for (Size i=0; i<n_; ++i) {
            weights_[i] = outstandings[i]/outstanding_;
            registerWith(quotes_[i]);
        }

    }


    RendistatoCalculator::RendistatoCalculator(
                            const boost::shared_ptr<RendistatoBasket>& basket,
                            const boost::shared_ptr<Euribor>& euriborIndex,
                            const Handle<YieldTermStructure>& discountCurve)
    : basket_(basket),
      euriborIndex_(euriborIndex), discountCurve_(discountCurve),
      yields_(basket_->size(), 0.05), durations_(basket_->size()) {
        registerWith(basket_);

        // TODO: generalize number of swaps and their lenghts
        Rate dummyRate = 0.05;
        for (Size i=0; i<10; ++i) {
            Natural lenght = i+2;
            swapLenghts_.push_back(lenght);
            boost::shared_ptr<VanillaSwap> swap =
                MakeVanillaSwap(lenght*Years, euriborIndex_, dummyRate, 1*Days).
                    withDiscountingTermStructure(discountCurve_);
            swaps_.push_back(swap);
        }
        nSwaps_ = swaps_.size();
    }

    void RendistatoCalculator::performCalculations() const {

        const std::vector<boost::shared_ptr<BTP> >& btps = basket_->btps();
        const std::vector<Handle<Quote> >& quotes = basket_->cleanPriceQuotes();
        for (Size i=0; i<basket_->size(); ++i) {
            yields_[i] = BondFunctions::yield(
                *btps[i], quotes[i]->value(),
                ActualActual(ActualActual::ISMA), Compounded, Annual,
                // settlementDate, accuracy, maxIterations, guess
                Date(), 1.0e-10, 100, yields_[i]); 
            durations_[i] = BondFunctions::duration(
                *btps[i], yields_[i],
                ActualActual(ActualActual::ISMA), Compounded, Annual,
                Duration::Modified, Date()); // settlementDate
        }
        duration_ = std::inner_product(basket_->weights().begin(),
                                       basket_->weights().end(),
                                       durations_.begin(), 0.0);

        equivalentSwapIndex_=nSwaps_-1;
        // starting from second swap
        for (Size i=1; i<nSwaps_; ++i) {
            Rate swapRate = swaps_[i]->fairRate();
            FixedRateBond swapBond(btps[i]->settlementDays(),
                                   100.0, // faceAmount
                                   swaps_[i]->fixedSchedule(),
                                   std::vector<Rate>(1, swapRate),
                                   swaps_[i]->fixedDayCount(),
                                   Following, 100.0); // paymentConvention, redemption
            // should be the NPV of the floating leg with end payment
            Real swapBondPrice = 100.0;
            Rate swapBondYield = BondFunctions::yield(
                swapBond, swapBondPrice,
                ActualActual(ActualActual::ISMA), Compounded, Annual,
                // settlementDate, accuracy, maxIterations, guess
                Date(), 1.0e-10, 100, swapRate);
            Time swapBondDuration = BondFunctions::duration(
                swapBond, swapBondYield,
                ActualActual(ActualActual::ISMA), Compounded, Annual,
                Duration::Modified, Date()); // settlementDate
            if (swapBondDuration > duration_) {
                equivalentSwapIndex_ = i-1;
                continue; // exit the loop
            }
        }

        return;
    }

    RendistatoEquivalentSwapLengthQuote::RendistatoEquivalentSwapLengthQuote(
        const boost::shared_ptr<RendistatoCalculator>& r) : r_(r) {}

    bool RendistatoEquivalentSwapLengthQuote::isValid() const {
        try {
            value();
            return true;
        } catch (...) {
            return false;
        }
    }

    RendistatoEquivalentSwapSpreadQuote::RendistatoEquivalentSwapSpreadQuote(
        const boost::shared_ptr<RendistatoCalculator>& r) : r_(r) {}

    bool RendistatoEquivalentSwapSpreadQuote::isValid() const {
        try {
            value();
            return true;
        } catch (...) {
            return false;
        }
    }
}
