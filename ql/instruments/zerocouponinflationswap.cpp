/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon
 Copyright (C) 2009 StatPro Italia srl

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

#include <ql/instruments/zerocouponinflationswap.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/indexedcashflow.hpp>

namespace QuantLib {

    /* Generally inflation indices are available with a lag of 1month
       and then observed with a lag of 2-3 months depending whether
       they use an interpolated fixing or not.  Here, we make the
       swap use the interpolation of the index to avoid incompatibilities.
    */
    ZeroCouponInflationSwap::ZeroCouponInflationSwap(
        Type type,
        Real nominal,
        const Date& startDate,  // start date of contract (only)
        const Date& maturity,   // this is pre-adjustment!
        const Calendar& fixCalendar,
        BusinessDayConvention fixConvention,
        const DayCounter& dayCounter,
        Rate fixedRate,
        const boost::shared_ptr<ZeroInflationIndex> &infIndex,
        const Period& observationLag,
        bool adjustInfObsDates,
        Calendar infCalendar,
        BusinessDayConvention infConvention)
    : Swap(2), type_(type), nominal_(nominal),
      startDate_(startDate), maturityDate_(maturity),
      fixCalendar_(fixCalendar), fixConvention_(fixConvention),
      fixedRate_(fixedRate), infIndex_(infIndex),
      observationLag_(observationLag), adjustInfObsDates_(adjustInfObsDates),
      infCalendar_(infCalendar), infConvention_(infConvention),
      dayCounter_(dayCounter) {
        // first check compatibility of index and swap definitions
        if (infIndex_->interpolated()) {
            Period pShift(infIndex_->frequency());
            QL_REQUIRE(observationLag_ - pShift > infIndex_->availabilityLag(),
                       "inconsistency between swap observation of index " << observationLag_ <<
                       " index availability " << infIndex_->availabilityLag() <<
                       " interpolated index period " << pShift <<
                       " and index availability " << infIndex_->availabilityLag() <<
                       " need (obsLag-index period) > availLag");
        } else {
            QL_REQUIRE(infIndex_->availabilityLag() < observationLag_,
                       "index tries to observe inflation fixings that do not yet exist: "
                       << " availability lag " << infIndex_->availabilityLag()
                       << " versus obs lag = " << observationLag_);
        }

        if (infCalendar_==Calendar()) infCalendar_ = fixCalendar_;
        if (infConvention_==BusinessDayConvention()) infConvention_ = fixConvention_;

        if (adjustInfObsDates_) {
            baseDate_ = infCalendar_.adjust(startDate - observationLag_, infConvention_);
            obsDate_ = infCalendar_.adjust(maturity - observationLag_, infConvention_);
        } else {
            baseDate_ = startDate - observationLag_;
            obsDate_ = maturity - observationLag_;
        }

        Date infPayDate = infCalendar_.adjust(maturity, infConvention_);
        Date fixedPayDate = fixCalendar_.adjust(maturity, fixConvention_);

        // At this point the index may not be able to forecast
        // i.e. do not want to force the existence of an inflation
        // term structure before allowing users to create instruments.
        Real T = inflationYearFraction(infIndex_->frequency(), infIndex_->interpolated(),
                                       dayCounter_, baseDate_, obsDate_);
        // N.B. the -1.0 is because swaps only exchange growth, not notionals as well
        Real fixedAmount = nominal * ( std::pow(1.0 + fixedRate, T) - 1.0 );

        legs_[0].push_back(boost::shared_ptr<CashFlow>(
            new SimpleCashFlow(fixedAmount, fixedPayDate)));
        bool growthOnly = true;
        legs_[1].push_back(boost::shared_ptr<CashFlow>(
            new IndexedCashFlow(nominal,infIndex,baseDate_,obsDate_,infPayDate,growthOnly)));

        for (Size j=0; j<2; ++j) {
            for (Leg::iterator i = legs_[j].begin(); i!= legs_[j].end(); ++i)
                registerWith(*i);
        }

        switch (type_) {
            case Payer:
                payer_[0] = +1.0;
                payer_[1] = -1.0;
                break;
            case Receiver:
                payer_[0] = -1.0;
                payer_[1] = +1.0;
                break;
            default:
                QL_FAIL("Unknown zero-inflation-swap type");
        }

    }



    void ZeroCouponInflationSwap::setupArguments(PricingEngine::arguments* args) const {
        Swap::setupArguments(args);
        // you don't actually need to do anything else because it is so simple
    }

    void ZeroCouponInflationSwap::arguments::validate() const {
        Swap::arguments::validate();
        // you don't actually need to do anything else because it is so simple
    }

    void ZeroCouponInflationSwap::fetchResults(const PricingEngine::results* r) const {
        Swap::fetchResults(r);
        // you don't actually need to do anything else because it is so simple
    }

    Real ZeroCouponInflationSwap::fairRate() const {
        // What does this mean before or after trade date?
        // Always means that NPV is zero for _this_ instrument
        // if it was created with _this_ rate
        // _knowing_ the time from base to obs (etc).

        boost::shared_ptr<IndexedCashFlow> icf =
        boost::dynamic_pointer_cast<IndexedCashFlow>(legs_[1].at(0));
        QL_REQUIRE(icf,"failed to downcast to IndexedCashFlow in ::fairRate()");

        // +1 because the IndexedCashFlow has growthOnly=true
        Real growth = icf->amount() / icf->notional() + 1.0;
        Real T = inflationYearFraction(infIndex_->frequency(),
                                       infIndex_->interpolated(),
                                       dayCounter_, baseDate_, obsDate_);

        return std::pow(growth,1.0/T) - 1.0;

        // we cannot use this simple definition because
        // it does not work for already-issued instruments
        // return infIndex_->zeroInflationTermStructure()->zeroRate(
        //      maturityDate(), observationLag(), infIndex_->interpolated());
    }


    Real ZeroCouponInflationSwap::fixedLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real ZeroCouponInflationSwap::inflationLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

    const Leg& ZeroCouponInflationSwap::fixedLeg() const {
        return legs_[0];
    }

    const Leg& ZeroCouponInflationSwap::inflationLeg() const {
        return legs_[1];
    }

}

