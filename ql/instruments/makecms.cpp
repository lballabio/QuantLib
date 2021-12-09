/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007, 2014 Ferdinando Ametrano

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

#include <ql/instruments/makecms.hpp>
#include <ql/instruments/swap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/daycounters/actual360.hpp>

namespace QuantLib {

    MakeCms::MakeCms(const Period& swapTenor,
                     const ext::shared_ptr<SwapIndex>& swapIndex,
                     const ext::shared_ptr<IborIndex>& iborIndex,
                     Spread iborSpread,
                     const Period& forwardStart)
    : swapTenor_(swapTenor), swapIndex_(swapIndex),
      iborIndex_(iborIndex), iborSpread_(iborSpread),
      useAtmSpread_(false), forwardStart_(forwardStart),

      cmsSpread_(0.0), cmsGearing_(1.0),
      cmsCap_(Null<Real>()), cmsFloor_(Null<Real>()),

      effectiveDate_(Date()),
      cmsCalendar_(swapIndex->fixingCalendar()),
      floatCalendar_(iborIndex->fixingCalendar()),
      payCms_(true), nominal_(1.0),
      cmsTenor_(3*Months), floatTenor_(iborIndex->tenor()),
      cmsConvention_(ModifiedFollowing),
      cmsTerminationDateConvention_(ModifiedFollowing),
      floatConvention_(iborIndex->businessDayConvention()),
      floatTerminationDateConvention_(iborIndex->businessDayConvention()),
      cmsRule_(DateGeneration::Backward), floatRule_(DateGeneration::Backward),
      cmsEndOfMonth_(false), floatEndOfMonth_(false),
      cmsFirstDate_(Date()), cmsNextToLastDate_(Date()),
      floatFirstDate_(Date()), floatNextToLastDate_(Date()),
      cmsDayCount_(Actual360()),
      floatDayCount_(iborIndex->dayCounter()),
      // arbitrary choice:
      //engine_(new DiscountingSwapEngine(iborIndex->termStructure())),
      engine_(new DiscountingSwapEngine(swapIndex->forwardingTermStructure())) {}


    MakeCms::MakeCms(const Period& swapTenor,
                     const ext::shared_ptr<SwapIndex>& swapIndex,
                     Spread iborSpread,
                     const Period& forwardStart)
    : swapTenor_(swapTenor), swapIndex_(swapIndex),
      iborIndex_(swapIndex->iborIndex()), iborSpread_(iborSpread),
      useAtmSpread_(false), forwardStart_(forwardStart),

      cmsSpread_(0.0), cmsGearing_(1.0),
      cmsCap_(Null<Real>()), cmsFloor_(Null<Real>()),

      effectiveDate_(Date()),
      cmsCalendar_(swapIndex->fixingCalendar()),
      floatCalendar_(iborIndex_->fixingCalendar()),
      payCms_(true), nominal_(1.0),
      cmsTenor_(3*Months), floatTenor_(iborIndex_->tenor()),
      cmsConvention_(ModifiedFollowing),
      cmsTerminationDateConvention_(ModifiedFollowing),
      floatConvention_(iborIndex_->businessDayConvention()),
      floatTerminationDateConvention_(iborIndex_->businessDayConvention()),
      cmsRule_(DateGeneration::Backward), floatRule_(DateGeneration::Backward),
      cmsEndOfMonth_(false), floatEndOfMonth_(false),
      cmsFirstDate_(Date()), cmsNextToLastDate_(Date()),
      floatFirstDate_(Date()), floatNextToLastDate_(Date()),
      cmsDayCount_(Actual360()),
      floatDayCount_(iborIndex_->dayCounter()),
      engine_(new DiscountingSwapEngine(swapIndex->forwardingTermStructure())) {}


    MakeCms::operator Swap() const {
        ext::shared_ptr<Swap> swap = *this;
        return *swap;
    }

    MakeCms::operator ext::shared_ptr<Swap>() const {

        Date startDate;
        if (effectiveDate_ != Date())
            startDate = effectiveDate_;
        else {
            Natural fixingDays = iborIndex_->fixingDays();
            Date refDate = Settings::instance().evaluationDate();
            // if the evaluation date is not a business day
            // then move to the next business day
            refDate = floatCalendar_.adjust(refDate);
            Date spotDate = floatCalendar_.advance(refDate,
                                                   fixingDays*Days);
            startDate = spotDate+forwardStart_;
        }

        Date terminationDate = startDate+swapTenor_;

        Schedule cmsSchedule(startDate, terminationDate,
                             cmsTenor_, cmsCalendar_,
                             cmsConvention_,
                             cmsTerminationDateConvention_,
                             cmsRule_, cmsEndOfMonth_,
                             cmsFirstDate_, cmsNextToLastDate_);

        Schedule floatSchedule(startDate, terminationDate,
                               floatTenor_, floatCalendar_,
                               floatConvention_,
                               floatTerminationDateConvention_,
                               floatRule_ , floatEndOfMonth_,
                               floatFirstDate_, floatNextToLastDate_);

        Leg cmsLeg = CmsLeg(cmsSchedule, swapIndex_)
            .withNotionals(nominal_)
            .withPaymentDayCounter(cmsDayCount_)
            .withPaymentAdjustment(cmsConvention_)
            .withFixingDays(swapIndex_->fixingDays())
            .withGearings(cmsGearing_)
            .withSpreads(cmsSpread_)
            .withCaps(cmsCap_)
            .withFloors(cmsFloor_);
        if (couponPricer_ != nullptr)
            setCouponPricer(cmsLeg, couponPricer_);

        Rate usedSpread = iborSpread_;
        if (useAtmSpread_) {
            QL_REQUIRE(!iborIndex_->forwardingTermStructure().empty(),
                       "null term structure set to this instance of " <<
                       iborIndex_->name());
            QL_REQUIRE(!swapIndex_->forwardingTermStructure().empty(),
                       "null term structure set to this instance of " <<
                       swapIndex_->name());
            QL_REQUIRE(couponPricer_,
                       "no CmsCouponPricer set (yet)");
            Leg floatLeg = IborLeg(floatSchedule, iborIndex_)
                .withNotionals(nominal_)
                .withPaymentDayCounter(floatDayCount_)
                .withPaymentAdjustment(floatConvention_)
                .withFixingDays(iborIndex_->fixingDays());

            Swap temp(cmsLeg, floatLeg);
            temp.setPricingEngine(engine_);

            Real npv = temp.legNPV(0)+temp.legNPV(1);

            usedSpread = -npv/temp.legBPS(1)*1e-4;
        } else {
            QL_REQUIRE(usedSpread != Null<Spread>(),
                       "null spread set");
        }

        Leg floatLeg = IborLeg(floatSchedule, iborIndex_)
            .withNotionals(nominal_)
            .withPaymentDayCounter(floatDayCount_)
            .withPaymentAdjustment(floatConvention_)
            .withFixingDays(iborIndex_->fixingDays())
            .withSpreads(usedSpread);

        ext::shared_ptr<Swap> swap;
        if (payCms_)
            swap = ext::make_shared<Swap>(cmsLeg, floatLeg);
        else
            swap = ext::make_shared<Swap>(floatLeg, cmsLeg);
        swap->setPricingEngine(engine_);
        return swap;
    }

    MakeCms& MakeCms::receiveCms(bool flag) {
        payCms_ = !flag;
        return *this;
    }

    MakeCms& MakeCms::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    MakeCms&
    MakeCms::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeCms& MakeCms::withDiscountingTermStructure(
                const Handle<YieldTermStructure>& discountingTermStructure) {
        engine_ = ext::make_shared<DiscountingSwapEngine>(discountingTermStructure);
        return *this;
    }

    MakeCms& MakeCms::withCmsCouponPricer(
                    const ext::shared_ptr<CmsCouponPricer>& couponPricer) {
        couponPricer_ = couponPricer;
        return *this;
    }

    MakeCms& MakeCms::withCmsLegTenor(const Period& t) {
        cmsTenor_ = t;
        return *this;
    }

    MakeCms&
    MakeCms::withCmsLegCalendar(const Calendar& cal) {
        cmsCalendar_ = cal;
        return *this;
    }

    MakeCms&
    MakeCms::withCmsLegConvention(BusinessDayConvention bdc) {
        cmsConvention_ = bdc;
        return *this;
    }

    MakeCms&
    MakeCms::withCmsLegTerminationDateConvention(BusinessDayConvention bdc) {
        cmsTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeCms& MakeCms::withCmsLegRule(DateGeneration::Rule r) {
        cmsRule_ = r;
        return *this;
    }

    MakeCms& MakeCms::withCmsLegEndOfMonth(bool flag) {
        cmsEndOfMonth_ = flag;
        return *this;
    }

    MakeCms& MakeCms::withCmsLegFirstDate(const Date& d) {
        cmsFirstDate_ = d;
        return *this;
    }

    MakeCms&
    MakeCms::withCmsLegNextToLastDate(const Date& d) {
        cmsNextToLastDate_ = d;
        return *this;
    }

    MakeCms&
    MakeCms::withCmsLegDayCount(const DayCounter& dc) {
        cmsDayCount_ = dc;
        return *this;
    }

    MakeCms& MakeCms::withFloatingLegTenor(const Period& t) {
        floatTenor_ = t;
        return *this;
    }

    MakeCms&
    MakeCms::withFloatingLegCalendar(const Calendar& cal) {
        floatCalendar_ = cal;
        return *this;
    }

    MakeCms&
    MakeCms::withFloatingLegConvention(BusinessDayConvention bdc) {
        floatConvention_ = bdc;
        return *this;
    }

    MakeCms&
    MakeCms::withFloatingLegTerminationDateConvention(BusinessDayConvention bdc) {
        floatTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeCms& MakeCms::withFloatingLegRule(DateGeneration::Rule r) {
        floatRule_ = r;
        return *this;
    }

    MakeCms& MakeCms::withFloatingLegEndOfMonth(bool flag) {
        floatEndOfMonth_ = flag;
        return *this;
    }

    MakeCms&
    MakeCms::withFloatingLegFirstDate(const Date& d) {
        floatFirstDate_ = d;
        return *this;
    }

    MakeCms&
    MakeCms::withFloatingLegNextToLastDate(const Date& d) {
        floatNextToLastDate_ = d;
        return *this;
    }

    MakeCms&
    MakeCms::withFloatingLegDayCount(const DayCounter& dc) {
        floatDayCount_ = dc;
        return *this;
    }

    MakeCms& MakeCms::withAtmSpread(bool flag) {
        useAtmSpread_ = flag;
        return *this;
    }

}
