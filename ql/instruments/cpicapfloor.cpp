/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010, 2011 Chris Kenyon
 Copyright (C) 2021 Ralf Konrad Eckel

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
 */


#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/instruments/cpicapfloor.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/schedule.hpp>
#include <utility>


namespace QuantLib {

    CPICapFloor::CPICapFloor(Option::Type type,
                             Real nominal,
                             const Date& startDate, // start date of contract (only)
                             Real baseCPI,
                             const Date& maturity, // this is pre-adjustment!
                             Calendar fixCalendar,
                             BusinessDayConvention fixConvention,
                             Calendar payCalendar,
                             BusinessDayConvention payConvention,
                             Rate strike,
                             ext::shared_ptr<ZeroInflationIndex>  index,
                             const Period& observationLag,
                             CPI::InterpolationType observationInterpolation)
    : type_(type), nominal_(nominal), startDate_(startDate), baseCPI_(baseCPI), maturity_(maturity),
      fixCalendar_(std::move(fixCalendar)), fixConvention_(fixConvention),
      payCalendar_(std::move(payCalendar)), payConvention_(payConvention), strike_(strike),
      index_(std::move(index)), observationLag_(observationLag),
      observationInterpolation_(observationInterpolation) {
        QL_REQUIRE(index_, "no inflation index passed");
        QL_REQUIRE(fixCalendar_ != Calendar(), "no fixing calendar passed");
        QL_REQUIRE(payCalendar_ != Calendar(), "no payment calendar passed");

        if (!detail::CPI::isInterpolated(observationInterpolation_)) {
            QL_REQUIRE(observationLag_ >= index_->availabilityLag(),
                       "CPIcapfloor's observationLag must be at least availabilityLag of inflation index: "
                       <<"when the observation is effectively flat"
                       << observationLag_ << " vs " << index_->availabilityLag());
        } else {
            QL_REQUIRE(observationLag_ > index_->availabilityLag(),
                       "CPIcapfloor's observationLag must be greater than availabilityLag of inflation index: "
                       <<"when the observation is effectively linear"
                       << observationLag_ << " vs " << index_->availabilityLag());
        }
    }


    // when you fix - but remember that there is an observation interpolation factor as well
    Date CPICapFloor::fixingDate() const {
        return fixCalendar_.adjust(maturity_ - observationLag_, fixConvention_);
    }


    Date CPICapFloor::payDate() const {
        return payCalendar_.adjust(maturity_, payConvention_);
    }


    bool CPICapFloor::isExpired() const {
        return (Settings::instance().evaluationDate() > maturity_);
    }


    void CPICapFloor::arguments::validate() const {
        // nothing yet
    }


    void CPICapFloor::setupArguments(PricingEngine::arguments* args) const {

        // correct PricingEngine?
        auto* arguments = dynamic_cast<CPICapFloor::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type, not CPICapFloor::arguments*");

        // data move
        arguments->type = type_;
        arguments->nominal = nominal_;
        arguments->startDate = startDate_;
        arguments->baseCPI = baseCPI_;
        arguments->maturity = maturity_;
        arguments->fixCalendar = fixCalendar_;
        arguments->fixConvention = fixConvention_;
        arguments->payCalendar = payCalendar_;
        arguments->payConvention = payConvention_;
        arguments->fixDate = fixingDate();
        arguments->payDate = payDate();
        arguments->strike = strike_;
        arguments->index = index_;
        arguments->observationLag = observationLag_;
        arguments->observationInterpolation = observationInterpolation_;
    }

}
