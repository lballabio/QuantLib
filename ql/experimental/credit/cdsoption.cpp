/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Stamm

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

#include <ql/experimental/credit/cdsoption.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    CdsOption::CdsOption(const Date& expiry,
                         Real strike,
                         const Handle<Quote>& volatility,
                         const Issuer& issuer,
                         Protection::Side side,
                         Real nominal,
                         const Schedule& premiumSchedule,
                         const DayCounter& dayCounter,
                         bool settlePremiumAccrual,
                         const Handle<YieldTermStructure>& yieldTS)
        : expiry_(expiry), strike_(strike), volatility_(volatility),
          issuer_(issuer), side_(side), nominal_(nominal),
          premiumSchedule_(premiumSchedule), dayCounter_(dayCounter),
          settlePremiumAccrual_(settlePremiumAccrual), yieldTS_(yieldTS) {
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        QL_REQUIRE(strike_ > 0, "Strike must be greater than 0");

        registerWith(volatility_);
        registerWith(issuer_.defaultProbability());
        registerWith(yieldTS_);
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    bool CdsOption::isExpired () const {
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        Date today = Settings::instance().evaluationDate();
        return (expiry_ <= today);
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    void CdsOption::setupExpired() const {
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        Instrument::setupExpired();
        forward_ = 0.0;
        riskyAnnuity_ = 0.0;
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Real CdsOption::forward() const {
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        calculate();
        return forward_;
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Real CdsOption::riskyAnnuity() const {
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        calculate();
        return riskyAnnuity_;
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    void CdsOption::performCalculations() const {
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        errorEstimate_ = Null<Real>();

        NPV_ = 0.0;
        riskyAnnuity_ = 0.0;
        forward_ = 0.0;
        Real defDiscProd = 0.0;
        Real defAnnuity = 0.0;
        Date today = Settings::instance().evaluationDate();
        Date settlement = yieldTS_->referenceDate();

        const Handle<DefaultProbabilityTermStructure>& defaultTS =
            issuer_.defaultProbability();
        for (Size i = 1; i < premiumSchedule_.size(); i++) {
            Date start = premiumSchedule_[i-1];
            Date end = premiumSchedule_[i];
            if (end > settlement) {
                Real discount = yieldTS_->discount(end);
                Real survProb = defaultTS->survivalProbability(end);
                Real defProb =
                    defaultTS->defaultProbability(std::max(start,today), end);
                Time dcf = dayCounter_.yearFraction(start,end);

                riskyAnnuity_ += discount * survProb * dcf;
                defAnnuity    += discount * defProb * dcf;
                defDiscProd   += discount * defProb;
            }
        }

        forward_ = (1 - issuer_.recoveryRate()) * defDiscProd
                 / (riskyAnnuity_ + 0.5 * defAnnuity);

        Real w = 0.0;
        switch (side_) {
          case Protection::Seller:
            w = -1.0;
            break;
          case Protection::Buyer:
            w = 1.0;
            break;
          default:
            QL_FAIL("unknown protection side");
        }

        Time expiryTime = dayCounter_.yearFraction(today, expiry_);
        Real stdDev = volatility_->value() * sqrt(expiryTime);

        Real d1 = std::log(forward_/strike_)/stdDev + 0.5 * stdDev;
        Real d2 = d1 - stdDev;

        NPV_ = riskyAnnuity_ * nominal_ *
               (w * forward_ * CumulativeNormalDistribution()(w * d1)
                - w * strike_ * CumulativeNormalDistribution()(w * d2));

    }

}

