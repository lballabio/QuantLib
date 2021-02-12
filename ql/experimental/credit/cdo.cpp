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

#include <ql/event.hpp>
#include <ql/experimental/credit/cdo.hpp>
#include <utility>

using namespace std;

namespace QuantLib {

    CDO::CDO(Real attachment,
             Real detachment,
             vector<Real> nominals,
             const vector<Handle<DefaultProbabilityTermStructure> >& basket,
             Handle<OneFactorCopula> copula,
             bool protectionSeller,
             Schedule premiumSchedule,
             Rate premiumRate,
             DayCounter dayCounter,
             Rate recoveryRate,
             Rate upfrontPremiumRate,
             Handle<YieldTermStructure> yieldTS,
             Size nBuckets,
             const Period& integrationStep)
    : attachment_(attachment), detachment_(detachment), nominals_(std::move(nominals)),
      basket_(basket), copula_(std::move(copula)), protectionSeller_(protectionSeller),
      premiumSchedule_(std::move(premiumSchedule)), premiumRate_(premiumRate),
      dayCounter_(std::move(dayCounter)), recoveryRate_(recoveryRate),
      upfrontPremiumRate_(upfrontPremiumRate), yieldTS_(std::move(yieldTS)), nBuckets_(nBuckets),
      integrationStep_(integrationStep) {

        QL_REQUIRE (!basket.empty(), "basket is empty");
        QL_REQUIRE (attachment_ >= 0 && attachment_ < detachment_
                    && detachment_ <= 1,
                    "illegal attachment/detachment point");

        registerWith (yieldTS_);
        registerWith (copula_);
        for (auto& i : basket_)
            registerWith(i);

        QL_REQUIRE (nominals_.size() <= basket_.size(),
                    "nominal vector size too large");

        if (nominals_.size() < basket_.size()) {
            Size n = basket_.size() - nominals_.size();
            Real back = nominals_.back();
            for (Size i = 0; i < n; i++)
                nominals_.push_back(back);
        }

        QL_REQUIRE (nominals_.size() == basket_.size(),
                    "nominal size " << nominals_.size()
                    << " != basket size " << basket_.size());

        nominal_ = 0;
        for (Size i = 0; i < nominals_.size(); i++) {
            lgds_.push_back (nominals_[i] * (1.0 - recoveryRate_));
            nominal_ += nominals_[i];
            lgd_ += lgds_[i];
        }
        xMax_ = detachment_ * nominal_;
        xMin_ = attachment_ * nominal_;
    }


    bool CDO::isExpired () const {
        return detail::simple_event(premiumSchedule_.dates().back())
               .hasOccurred(yieldTS_->referenceDate());
    }


    void CDO::setupExpired() const {
        Instrument::setupExpired();
    }


    Real CDO::expectedTrancheLoss (Date d) const {
        if (d <= basket_.front()->referenceDate())
            return 0;

        vector<Real> defProb (basket_.size());
        for (Size j = 0; j < basket_.size(); j++)
            defProb[j] = basket_[j]->defaultProbability (d);

        LossDistBucketing op (nBuckets_, xMax_);
        Distribution dist = copula_->integral (op, lgds_, defProb);

        return dist.trancheExpectedValue (xMin_, xMax_);

        // The following causes two errors in test against literature values.
        // FIXME: Investigate accuracy.
        // return dist.cumulativeExcessProbability (xMin_, xMax_);

        //   TranchePayoff func (xMin_, xMax_);
        //   return (dist.expectedValue (func)
        //    + (xMax_ - xMin_) * (1.0 - dist.cumulatedProbability (xMax_)));
    }


    void CDO::performCalculations() const {

        QL_REQUIRE(!yieldTS_.empty(), "no yield term structure set");

        errorEstimate_ = Null<Real>();

        NPV_ = 0.0;
        premiumValue_ = 0;
        protectionValue_ = 0;
        error_ = 0;

        /* Expectations e1 and e2 are portfolio loss given default,
           i.e.  with recovery already "bult in". Multiplication by
           (1-r) is therefore not necessary, neither in premium nor
           protection value calculation.
        */

        Real e1 = 0;
        Date today = yieldTS_->referenceDate();
        if (premiumSchedule_[0] > today)
            e1 = expectedTrancheLoss (premiumSchedule_[0]);

        for (Size i = 1; i < premiumSchedule_.size(); i++) {
            Date d2 = premiumSchedule_[i];
            if (d2 < today)
                continue;

            Date d1 = premiumSchedule_[i-1];

            Date d, d0 = d1;
            do {
                d = NullCalendar().advance (d0 > today ? d0 : today,
                                            integrationStep_);
                if (d > d2)
                    d = d2;

                Real e2 = expectedTrancheLoss (d);

                premiumValue_ += (xMax_ - xMin_ - e2)
                    * premiumRate_ * dayCounter_.yearFraction (d0, d)
                    * yieldTS_->discount (d);

                if (e2 < e1) {
                    error_ ++;
                }

                protectionValue_ -= (e2 - e1) * yieldTS_->discount (d);

                d0 = d;
                e1 = e2;
            }
            while (d < d2);
        }

        if (premiumSchedule_[0] >= today)
            upfrontPremiumValue_ = (xMax_ - xMin_) * upfrontPremiumRate_ *
                yieldTS_->discount(premiumSchedule_[0]);
        else
            upfrontPremiumValue_ = 0.0;

        if (!protectionSeller_) {
            premiumValue_ *= -1;
            upfrontPremiumValue_ *= -1;
            protectionValue_ *= -1;
        }

        NPV_ = premiumValue_ + protectionValue_ + upfrontPremiumValue_;
    }


    Rate CDO::premiumValue () const {
        calculate();
        return premiumValue_;
    }

    Rate CDO::protectionValue () const {
        calculate();
        return protectionValue_;
    }

    Size CDO::error () const {
        calculate();
        return error_;
    }

    Rate CDO::fairPremium () const {
        calculate();
        return - premiumRate_ * protectionValue_ / premiumValue_;
    }

}
