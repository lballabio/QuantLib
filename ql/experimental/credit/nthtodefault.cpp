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

namespace QuantLib {


    NthToDefault::NthToDefault(Size n,
                               const std::vector<Issuer>& basket,
                               const Handle<OneFactorCopula>& copula,
                               Protection::Side side,
                               Real nominal,
                               const Schedule& premiumSchedule,
                               Rate premiumRate,
                               const DayCounter& dayCounter,
                               bool settlePremiumAccrual,
                               const Handle<YieldTermStructure>& yieldTS,
                               const Period& integrationStepSize,
                               boost::shared_ptr<Claim> claim)
    : n_(n), basket_(basket), copula_(copula),
      side_(side), nominal_(nominal),
      premiumSchedule_(premiumSchedule), premiumRate_(premiumRate),
      dayCounter_(dayCounter), settlePremiumAccrual_(settlePremiumAccrual),
      yieldTS_(yieldTS), integrationStepSize_(integrationStepSize),
      claim_(claim) {

        premiumLeg_ = FixedRateLeg(premiumSchedule, dayCounter)
            .withNotionals(nominal)
            .withCouponRates(premiumRate)
            .withPaymentAdjustment(Unadjusted);

        for (Size i=0; i<basket_.size(); i++)
            registerWith(basket_[i]);
        registerWith(copula_);
        registerWith(yieldTS_);

        if (!claim_)
            claim_ = boost::shared_ptr<Claim>(new FaceValueClaim);
        registerWith(claim_);
    }

    bool NthToDefault::isExpired() const {
        Date settlement = yieldTS_->referenceDate();
        return (premiumLeg_.back()->date() <= settlement);
    }

    Rate NthToDefault::fairPremium() const {
        calculate();
        QL_REQUIRE(fairPremium_ != Null<Rate>(),
                   "fair premium not available");
        return fairPremium_;
    }

    void NthToDefault::setupExpired() const {
        Instrument::setupExpired();
        fairPremium_ = 0.0;
    }

    void NthToDefault::performCalculations() const {
        QL_REQUIRE(!yieldTS_.empty(), "no yield term structure set");

        for (Size i = 0; i < basket_.size(); i++) {
            QL_REQUIRE(!basket_[i].defaultProbability().empty(),
                       "no default term structure set");
            if (i > 0) {
                QL_REQUIRE(basket_[i].defaultProbability()->referenceDate() ==
                           basket_[i-1].defaultProbability()->referenceDate(),
                           "basket reference dates do not match");
                QL_REQUIRE(
                     basket_[i].recoveryRate() == basket_[i-1].recoveryRate(),
                     "basket recovery rates do not match");
            }
        }

        errorEstimate_ = Null<Real>();

        NPV_ = 0.0;
        Real premiumValue = 0.0;
        Real accrualValue = 0.0;
        Real claimValue = 0.0;
        Real recoveryRate = basket_.front().recoveryRate();
        Date d, d0;
        for (Size i = 0; i < premiumLeg_.size(); i++) {
            boost::shared_ptr<FixedRateCoupon> coupon =
                boost::dynamic_pointer_cast<FixedRateCoupon>(premiumLeg_[i]);
            Date d = premiumLeg_[i]->date();
            if (d > yieldTS_->referenceDate()) {
                premiumValue += premiumLeg_[i]->amount()
                    * yieldTS_->discount(d)
                    * (1.0 - defaultProbability(d));

                if (coupon->accrualStartDate() >= yieldTS_->referenceDate())
                    d = coupon->accrualStartDate();
                else
                    d = yieldTS_->referenceDate();

                // do steps of specified size
                d0 = d;
                Period stepSize = integrationStepSize_;
                Probability defProb0 = defaultProbability(d0);
                do {
                    DiscountFactor disc = yieldTS_->discount(d);
                    Probability defProb1 = defaultProbability(d);
                    Probability dcfdd = defProb1 - defProb0;

                    defProb0 = defProb1;

                    if (settlePremiumAccrual_)
                        accrualValue += coupon->accruedAmount(d)*disc*dcfdd;

                    claimValue -= claim_->amount(d, nominal_,
                                                 recoveryRate)
                                  * disc * dcfdd;

                    d0 = d;
                    d = d0 + stepSize;
                    // reduce step size ?
                    if (stepSize != 1*Days && d > coupon->accrualEndDate()) {
                        stepSize = 1*Days;
                        //d = NullCalendar().advance(d0,1,stepSize,Unadjusted);
                        d = d0 + stepSize;
                    }
                }
                while (d <= coupon->accrualEndDate());
            }
        }

        if (side_ == Protection::Buyer) {
            premiumValue *= -1;
            accrualValue *= -1;
            claimValue *= -1;
        }

        NPV_ = premiumValue + accrualValue + claimValue;

        fairPremium_ = -premiumRate_*claimValue/(premiumValue+accrualValue);
    }

    Probability NthToDefault::defaultProbability(const Date& d) const {
        if (d <= basket_.front().defaultProbability()->referenceDate())
            return 0.0;

        std::vector<Real> defProb(basket_.size());
        for (Size j = 0; j < basket_.size(); j++)
            defProb[j] = basket_[j].defaultProbability()->defaultProbability(d);

        ProbabilityOfAtLeastNEvents op(n_);

        return copula_->integral(op, defProb);
    }

}
