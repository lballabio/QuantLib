/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Instruments/makecms.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/DayCounters/actual360.hpp>

namespace QuantLib {

    MakeCMS::MakeCMS(const Period& swapTenor,
                     const boost::shared_ptr<SwapIndex>& swapIndex,
                     Spread iborSpread,
                     const boost::shared_ptr<VanillaCMSCouponPricer>& pricer,
                     const Period& forwardStart)
    : swapTenor_(swapTenor), swapIndex_(swapIndex),
      iborSpread_(iborSpread),
      cmsVanillapricer_(pricer), 
      forwardStart_(forwardStart),

      cmsSpread_(0.0), cmsGearing_(1.0),
      cmsCap_(2.0), cmsFloor_(0.0),

      effectiveDate_(Date()),
      cmsCalendar_(swapIndex->calendar()),
      floatCalendar_(swapIndex->iborIndex()->calendar()),

      discountingTermStructure_(swapIndex->termStructureHandle()),

      payCMS_(true), nominal_(1000000.0),
      cmsTenor_(3*Months), floatTenor_(3*Months),
      cmsConvention_(ModifiedFollowing),
      cmsTerminationDateConvention_(ModifiedFollowing),
      floatConvention_(ModifiedFollowing),
      floatTerminationDateConvention_(ModifiedFollowing),
      cmsBackward_(true), floatBackward_(true),
      cmsEndOfMonth_(false), floatEndOfMonth_(false),
      cmsFirstDate_(Date()), cmsNextToLastDate_(Date()),
      floatFirstDate_(Date()), floatNextToLastDate_(Date()),
      cmsDayCount_(Actual360()),
      floatDayCount_(swapIndex->iborIndex()->dayCounter())
    {
        boost::shared_ptr<IborIndex> baseIndex = swapIndex->iborIndex();
        // FIXME use a familyName-based index factory
        iborIndex_ = boost::shared_ptr<IborIndex>(new
            IborIndex(baseIndex->familyName(),
                  floatTenor_,
                  baseIndex->settlementDays(),
                  baseIndex->currency(),
                  baseIndex->calendar(),
                  baseIndex->businessDayConvention(),
                  baseIndex->endOfMonth(),
                  baseIndex->dayCounter(),
                  baseIndex->termStructureHandle()));
      }

    MakeCMS::operator Swap() const {

        Date startDate;
        if (effectiveDate_ != Date())
            startDate=effectiveDate_;
        else {
          Integer fixingDays = swapIndex_->settlementDays();
          Date referenceDate = Settings::instance().evaluationDate();
          Date spotDate = floatCalendar_.advance(referenceDate, fixingDays*Days);
          startDate = spotDate+forwardStart_;
        }

        Date terminationDate = startDate+swapTenor_;

        Schedule cmsSchedule(startDate, terminationDate,
                             cmsTenor_, cmsCalendar_,
                             cmsConvention_,
                             cmsTerminationDateConvention_,
                             cmsBackward_, cmsEndOfMonth_,
                             cmsFirstDate_, cmsNextToLastDate_);

        Schedule floatSchedule(startDate, terminationDate,
                               floatTenor_, floatCalendar_,
                               floatConvention_,
                               floatTerminationDateConvention_,
                               floatBackward_, floatEndOfMonth_,
                               floatFirstDate_, floatNextToLastDate_);

        std::vector<boost::shared_ptr<CashFlow> > cmsLeg =
            CMSCouponVector(cmsSchedule,
                            cmsConvention_,
                            std::vector<Real>(1, nominal_),
                            swapIndex_,
                            swapIndex_->settlementDays(),
                            cmsDayCount_,
                            std::vector<Real>(1, cmsGearing_),
                            std::vector<Spread>(1, cmsSpread_),
                            std::vector<Rate>(1, cmsCap_),
                            std::vector<Rate>(1, cmsFloor_),
                            cmsVanillapricer_);

        std::vector<boost::shared_ptr<CashFlow> > floatLeg =
            FloatingRateCouponVector(floatSchedule,
                                     floatConvention_,
                                     std::vector<Real>(1, nominal_),
                                     iborIndex_->settlementDays(),
                                     iborIndex_,
                                     std::vector<Real>(1, 1.0), // gearing
                                     std::vector<Spread>(1, iborSpread_),
                                     floatDayCount_);
        if (payCMS_)
            return Swap(discountingTermStructure_, cmsLeg, floatLeg);
        else
            return Swap(discountingTermStructure_, floatLeg, cmsLeg);
    }

    MakeCMS::operator boost::shared_ptr<Swap>() const {

        Date startDate;
        if (effectiveDate_ != Date())
            startDate=effectiveDate_;
        else {
          Integer fixingDays = swapIndex_->settlementDays();
          Date referenceDate = Settings::instance().evaluationDate();
          Date spotDate = floatCalendar_.advance(referenceDate, fixingDays*Days);
          startDate = spotDate+forwardStart_;
        }

        Date terminationDate = startDate+swapTenor_;

        Schedule cmsSchedule(startDate, terminationDate,
                             cmsTenor_, cmsCalendar_,
                             cmsConvention_,
                             cmsTerminationDateConvention_,
                             cmsBackward_, cmsEndOfMonth_,
                             cmsFirstDate_, cmsNextToLastDate_);

        Schedule floatSchedule(startDate, terminationDate,
                               floatTenor_, floatCalendar_,
                               floatConvention_,
                               floatTerminationDateConvention_,
                               floatBackward_, floatEndOfMonth_,
                               floatFirstDate_, floatNextToLastDate_);

        std::vector<boost::shared_ptr<CashFlow> > cmsLeg =
            CMSCouponVector(cmsSchedule,
                            cmsConvention_,
                            std::vector<Real>(1, nominal_),
                            swapIndex_,
                            swapIndex_->settlementDays(),
                            cmsDayCount_,
                            std::vector<Real>(1, cmsGearing_),
                            std::vector<Spread>(1, cmsSpread_),
                            std::vector<Rate>(1, cmsCap_),
                            std::vector<Rate>(1, cmsFloor_),
                            cmsVanillapricer_);

        std::vector<boost::shared_ptr<CashFlow> > floatLeg =
            FloatingRateCouponVector(floatSchedule,
                                     floatConvention_,
                                     std::vector<Real>(1, nominal_),
                                     iborIndex_->settlementDays(),
                                     iborIndex_,
                                     std::vector<Real>(1, 1.0), // gearing
                                     std::vector<Spread>(1, iborSpread_),
                                     floatDayCount_);
        if (payCMS_)
            return boost::shared_ptr<Swap>(new
                Swap(discountingTermStructure_, cmsLeg, floatLeg));
        else
            return boost::shared_ptr<Swap>(new
                Swap(discountingTermStructure_, floatLeg, cmsLeg));
    }

    MakeCMS& MakeCMS::receiveCMS(bool flag) {
        payCMS_ = !flag;
        return *this;
    }

    MakeCMS& MakeCMS::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    MakeCMS&
    MakeCMS::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeCMS& MakeCMS::withDiscountingTermStructure(
                const Handle<YieldTermStructure>& discountingTermStructure) {
        discountingTermStructure_ = discountingTermStructure;
        return *this;
    }

    MakeCMS& MakeCMS::withCMSLegTenor(const Period& t) {
        cmsTenor_ = t;
        return *this;
    }

    MakeCMS&
    MakeCMS::withCMSLegCalendar(const Calendar& cal) {
        cmsCalendar_ = cal;
        return *this;
    }

    MakeCMS&
    MakeCMS::withCMSLegConvention(BusinessDayConvention bdc) {
        cmsConvention_ = bdc;
        return *this;
    }

    MakeCMS&
    MakeCMS::withCMSLegTerminationDateConvention(BusinessDayConvention bdc) {
        cmsTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeCMS& MakeCMS::withCMSLegForward(bool flag) {
        cmsBackward_ = !flag;
        return *this;
    }

    MakeCMS& MakeCMS::withCMSLegEndOfMonth(bool flag) {
        cmsEndOfMonth_ = flag;
        return *this;
    }

    MakeCMS& MakeCMS::withCMSLegFirstDate(const Date& d) {
        cmsFirstDate_ = d;
        return *this;
    }

    MakeCMS&
    MakeCMS::withCMSLegNextToLastDate(const Date& d) {
        cmsNextToLastDate_ = d;
        return *this;
    }

    MakeCMS&
    MakeCMS::withCMSLegDayCount(const DayCounter& dc) {
        cmsDayCount_ = dc;
        return *this;
    }

    MakeCMS& MakeCMS::withFloatingLegTenor(const Period& t) {
        floatTenor_ = t;
        return *this;
    }

    MakeCMS&
    MakeCMS::withFloatingLegCalendar(const Calendar& cal) {
        floatCalendar_ = cal;
        return *this;
    }

    MakeCMS&
    MakeCMS::withFloatingLegConvention(BusinessDayConvention bdc) {
        floatConvention_ = bdc;
        return *this;
    }

    MakeCMS&
    MakeCMS::withFloatingLegTerminationDateConvention(BusinessDayConvention bdc) {
        floatTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeCMS& MakeCMS::withFloatingLegForward(bool flag) {
        floatBackward_ = !flag;
        return *this;
    }

    MakeCMS& MakeCMS::withFloatingLegEndOfMonth(bool flag) {
        floatEndOfMonth_ = flag;
        return *this;
    }

    MakeCMS&
    MakeCMS::withFloatingLegFirstDate(const Date& d) {
        floatFirstDate_ = d;
        return *this;
    }

    MakeCMS&
    MakeCMS::withFloatingLegNextToLastDate(const Date& d) {
        floatNextToLastDate_ = d;
        return *this;
    }

    MakeCMS&
    MakeCMS::withFloatingLegDayCount(const DayCounter& dc) {
        floatDayCount_ = dc;
        return *this;
    }

}
