/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010, 2011 Ferdinando Ametrano

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
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/schedule.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {

    CCTEU::CCTEU(const Date& maturityDate,
                 Spread spread,
                 const Handle<YieldTermStructure>& fwdCurve,
                 const Date& startDate,
                 const Date& issueDate)
    : FloatingRateBond(2, 100.0,
                       Schedule(startDate,
                                maturityDate, 6*Months,
                                NullCalendar(), Unadjusted, Unadjusted,
                                DateGeneration::Backward, true),
                       ext::make_shared<Euribor6M>(fwdCurve),
                       Actual360(),
                       Following,
                       Euribor6M().fixingDays(),
                       std::vector<Real>(1, 1.0), // gearing
                       std::vector<Spread>(1, spread),
                       std::vector<Rate>(), // caps
                       std::vector<Rate>(), // floors
                       false, // in arrears
                       100.0, // redemption
                       issueDate) {}

    BTP::BTP(const Date& maturityDate,
             Rate fixedRate,
             const Date& startDate,
             const Date& issueDate)
    : FixedRateBond(2, 100.0,
                    Schedule(startDate,
                             maturityDate, 6*Months,
                             NullCalendar(), Unadjusted, Unadjusted,
                             DateGeneration::Backward, true),
                    std::vector<Rate>(1, fixedRate),
                    ActualActual(ActualActual::ISMA),
                    ModifiedFollowing, 100.0, issueDate, TARGET()) {}

    BTP::BTP(const Date& maturityDate,
             Rate fixedRate,
             Real redemption,
             const Date& startDate,
             const Date& issueDate)
    : FixedRateBond(2, 100.0,
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


    RendistatoBasket::RendistatoBasket(const std::vector<ext::shared_ptr<BTP> >& btps,
                                       const std::vector<Real>& outstandings,
                                       std::vector<Handle<Quote> > cleanPriceQuotes)
    : btps_(btps), outstandings_(outstandings), quotes_(std::move(cleanPriceQuotes)) {

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


    RendistatoCalculator::RendistatoCalculator(ext::shared_ptr<RendistatoBasket> basket,
                                               ext::shared_ptr<Euribor> euriborIndex,
                                               Handle<YieldTermStructure> discountCurve)
    : basket_(std::move(basket)), euriborIndex_(std::move(euriborIndex)),
      discountCurve_(std::move(discountCurve)), yields_(basket_->size(), 0.05),
      durations_(basket_->size()),
      nSwaps_(15), // TODO: generalize number of swaps and their lengths
      swaps_(nSwaps_), swapLengths_(nSwaps_), swapBondDurations_(nSwaps_, Null<Time>()),
      swapBondYields_(nSwaps_, 0.05), swapRates_(nSwaps_, Null<Rate>()) {
        registerWith(basket_);
        registerWith(euriborIndex_);
        registerWith(discountCurve_);

        Rate dummyRate = 0.05;
        for (Size i=0; i<nSwaps_; ++i) {
            swapLengths_[i] = static_cast<Real>(i+1);
            swaps_[i] = MakeVanillaSwap(
                swapLengths_[i]*Years, euriborIndex_, dummyRate, 1*Days)
                                .withDiscountingTermStructure(discountCurve_);
        }
    }

    void RendistatoCalculator::performCalculations() const {

        const std::vector<ext::shared_ptr<BTP> >& btps = basket_->btps();
        const std::vector<Handle<Quote> >& quotes = basket_->cleanPriceQuotes();
        Date bondSettlementDate = btps[0]->settlementDate();
        for (Size i=0; i<basket_->size(); ++i) {
            yields_[i] = BondFunctions::yield(
                *btps[i], quotes[i]->value(),
                ActualActual(ActualActual::ISMA), Compounded, Annual,
                bondSettlementDate,
                // accuracy, maxIterations, guess
                1.0e-10, 100, yields_[i]);
            durations_[i] = BondFunctions::duration(
                *btps[i], yields_[i],
                ActualActual(ActualActual::ISMA), Compounded, Annual,
                Duration::Modified, bondSettlementDate);
        }
        duration_ = std::inner_product(basket_->weights().begin(),
                                       basket_->weights().end(),
                                       durations_.begin(), Real(0.0));

        Natural settlDays = 2;
        DayCounter fixedDayCount = swaps_[0]->fixedDayCount();
        equivalentSwapIndex_ = nSwaps_-1;
        swapRates_[0]= swaps_[0]->fairRate();
        FixedRateBond swapBond(settlDays,
                               100.0,      // faceAmount
                               swaps_[0]->fixedSchedule(),
                               std::vector<Rate>(1, swapRates_[0]),
                               fixedDayCount,
                               Following, // paymentConvention
                               100.0);    // redemption
        swapBondYields_[0] = BondFunctions::yield(swapBond,
            100.0, // floating leg NPV including end payment
            ActualActual(ActualActual::ISMA), Compounded, Annual,
            bondSettlementDate,
            // accuracy, maxIterations, guess
            1.0e-10, 100, swapBondYields_[0]);
        swapBondDurations_[0] = BondFunctions::duration(
            swapBond, swapBondYields_[0],
            ActualActual(ActualActual::ISMA), Compounded, Annual,
            Duration::Modified, bondSettlementDate);
        for (Size i=1; i<nSwaps_; ++i) {
            swapRates_[i]= swaps_[i]->fairRate();
            FixedRateBond swapBond(settlDays,
                                   100.0,      // faceAmount
                                   swaps_[i]->fixedSchedule(),
                                   std::vector<Rate>(1, swapRates_[i]),
                                   fixedDayCount,
                                   Following, // paymentConvention
                                   100.0);    // redemption
            swapBondYields_[i] = BondFunctions::yield(swapBond,
                100.0, // floating leg NPV including end payment
                ActualActual(ActualActual::ISMA), Compounded, Annual,
                bondSettlementDate,
                // accuracy, maxIterations, guess
                1.0e-10, 100, swapBondYields_[i]);
            swapBondDurations_[i] = BondFunctions::duration(
                swapBond, swapBondYields_[i],
                ActualActual(ActualActual::ISMA), Compounded, Annual,
                Duration::Modified, bondSettlementDate);
            if (swapBondDurations_[i] > duration_) {
                equivalentSwapIndex_ = i-1;
                break; // exit the loop
            }
        }
    }

    RendistatoEquivalentSwapLengthQuote::RendistatoEquivalentSwapLengthQuote(
        ext::shared_ptr<RendistatoCalculator> r)
    : r_(std::move(r)) {}

    bool RendistatoEquivalentSwapLengthQuote::isValid() const {
        try {
            value();
            return true;
        } catch (...) {
            return false;
        }
    }

    RendistatoEquivalentSwapSpreadQuote::RendistatoEquivalentSwapSpreadQuote(
        ext::shared_ptr<RendistatoCalculator> r)
    : r_(std::move(r)) {}

    bool RendistatoEquivalentSwapSpreadQuote::isValid() const {
        try {
            value();
            return true;
        } catch (...) {
            return false;
        }
    }
}
