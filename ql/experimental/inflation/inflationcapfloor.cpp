/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

#include <ql/experimental/inflation/inflationcapfloor.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    YoYInflationCapFloor::YoYInflationCapFloor(
                                          YoYInflationCapFloor::Type type,
                                          const YoYInflationLeg& floatingLeg,
                                          const std::vector<Rate>& capRates,
                                          const std::vector<Rate>& floorRates)
    : type_(type), floatingLeg_(floatingLeg),
      capRates_(capRates), floorRates_(floorRates) {

        if (type_ == Cap || type_ == Collar) {
            QL_REQUIRE(!capRates_.empty(), "no cap rates given");
            QL_REQUIRE(capRates_.size() == floatingLeg_.size(),
                       "number of cap rates different "
                       "from number of cashflows in leg"
                       << capRates_.size() << " rates, versus "
                       <<  floatingLeg_.size() << " coupons");  }
        if (type_ == Floor || type_ == Collar) {
            QL_REQUIRE(!floorRates_.empty(), "no floor rates given");
            QL_REQUIRE(floorRates_.size() == floatingLeg_.size(),
                       "number of floor rates different "
                       "from number of cashflows in leg"
                       << floorRates_.size() << " rates, versus "
                       <<  floatingLeg_.size() << " coupons");
        }

        YoYInflationLeg::const_iterator i;
        for (i = floatingLeg_.begin(); i != floatingLeg_.end(); ++i)
            registerWith(*i);

        registerWith(Settings::instance().evaluationDate());
    }


    YoYInflationCapFloor::YoYInflationCapFloor(
                                           YoYInflationCapFloor::Type type,
                                           const YoYInflationLeg& floatingLeg,
                                           const std::vector<Rate>& strikes)
    : type_(type), floatingLeg_(floatingLeg) {

        QL_REQUIRE(!strikes.empty(), "no strikes given");
        QL_REQUIRE(strikes.size() == floatingLeg_.size(),
                   "number of strikes different from number of cashflows in leg"
                   << strikes.size() << " strikes, versus "
                   <<  floatingLeg_.size() << " coupons");
        if (type_ == Cap) {
            capRates_ = strikes;
        } else if (type_ == Floor) {
            floorRates_ = strikes;
        } else
            QL_FAIL("only Cap/Floor types allowed in this constructor");

        YoYInflationLeg::const_iterator i;
        for (i = floatingLeg_.begin(); i != floatingLeg_.end(); ++i)
            registerWith(*i);

        registerWith(Settings::instance().evaluationDate());
    }


    bool YoYInflationCapFloor::isExpired() const {
        Date today = Settings::instance().evaluationDate();
        for (Size i=0; i<floatingLeg_.size(); i++)
            if (!floatingLeg_[i]->hasOccurred(today))
                return false;
        return true;
    }


    Date YoYInflationCapFloor::startDate() const {
        Size n = floatingLeg_.size();   // probably the slowest possible way
        Leg temp(n);
        for (Size i = 0; i < n; i++)
            temp[i] = boost::dynamic_pointer_cast<CashFlow>(floatingLeg_[i]);
        return CashFlows::startDate(temp);
    }


    Date YoYInflationCapFloor::maturityDate() const {
        Size n = floatingLeg_.size(); // probably the slowest possible way
        Leg temp(n);
        for (Size i = 0; i < n; i++)
            temp[i] = boost::dynamic_pointer_cast<CashFlow>(floatingLeg_[i]);
        return CashFlows::maturityDate(temp);
    }


    std::vector<Date> YoYInflationCapFloor::fixingDates() const {
        std::vector<Date> dates;  // must return by value as on stack
        for (Size i = 0; i < floatingLeg_.size(); i++)
            dates.push_back(floatingLeg_[i]->fixingDate());
        return dates;
    }


    std::vector<Date> YoYInflationCapFloor::paymentDates() const {
        std::vector<Date> dates;  // must return by value as on stack
        for (Size i = 0; i < floatingLeg_.size(); i++)
            dates.push_back(floatingLeg_[i]->date());
        return dates;
    }


    void YoYInflationCapFloor::setupArguments(
                                       PricingEngine::arguments* args) const {
        YoYInflationCapFloor::arguments* arguments =
            dynamic_cast<YoYInflationCapFloor::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        Size n = floatingLeg_.size();

        arguments->capRates.resize(n);
        arguments->floorRates.resize(n);

        arguments->type = type_;
        arguments->floatingLeg = floatingLeg_;

        Date today = Settings::instance().evaluationDate();

        for (Size i=0; i<n; i++) {
            if (type_ == Cap || type_ == Collar)
                arguments->capRates[i] = capRates_[i];
            else
                arguments->capRates[i] = Null<Rate>();

            if (type_ == Floor || type_ == Collar)
                arguments->floorRates[i] = floorRates_[i];
            else
                arguments->floorRates[i] = Null<Rate>();
        }
    }


    void YoYInflationCapFloor::arguments::validate() const {
        // given the checks in the constructor there is nothing to do
    }


    std::ostream& operator<<(std::ostream& out, YoYInflationCapFloor::Type t) {
        switch (t) {
          case YoYInflationCapFloor::Cap:
            return out << "Cap";
          case YoYInflationCapFloor::Floor:
            return out << "Floor";
          case YoYInflationCapFloor::Collar:
            return out << "Collar";
          default:
            QL_FAIL("unknown YoYInflationCapFloor::Type ("
                    << Integer(t) << ")");
        }
    }



    MakeYoYInflationCapFloor::MakeYoYInflationCapFloor(
                            YoYInflationCapFloor::Type capFloorType,
                            Period &lag, Natural fixingDays,
                            const boost::shared_ptr<YoYInflationIndex>& index,
                            Rate strike, Size n)
    : capFloorType_(capFloorType), asOptionlet_(false),
      lag_(lag), fixingDays_(fixingDays),
      index_(index), strike_(strike),
      tenorN_(n), nominal_(1.0), cal_(TARGET()),
      bdc_(ModifiedFollowing), dc_(Actual365Fixed()) {
        // just store everything until needed
        effectiveDate_ = Settings::instance().evaluationDate();
    }


    MakeYoYInflationCapFloor::
    operator YoYInflationCapFloor() const {
        boost::shared_ptr<YoYInflationCapFloor> yoycapfloor = *this;
        return *yoycapfloor;
    }

    MakeYoYInflationCapFloor::
    operator boost::shared_ptr<YoYInflationCapFloor>() const {

        QL_REQUIRE(tenorN_>0,"requested cap/floor with <1 coupon: "<< tenorN_);
        // cap with n coupons ...
        Schedule s = MakeSchedule().from(effectiveDate_)
                                   .to(effectiveDate_+Period(tenorN_,Years))
                                   .withTenor(Period(1,Years))
                                   .withCalendar(cal_)
                                   .withConvention(bdc_);
        // check you have the right number of payoffs
        QL_REQUIRE(s.size() == tenorN_+1,
                   "Dates/calendar/adjustment incompatible with "
                   " tenor/MakeSchedule: so made wrong number of coupon dates "
                   << (s.size()-1) <<" versus requested "<< tenorN_);

        // make really sure that you have correct defaults for "none"
        Rate cap = Null<Rate>();
        Rate floor = Null<Rate>();
        if (capFloorType_ == YoYInflationCapFloor::Cap)
            cap = strike_;
        else
            floor = strike_;
        Size useN = tenorN_;
        if (asOptionlet_) useN = 1;
        std::vector<Rate> capRates(useN, cap);
        std::vector<Rate> floorRates(useN, floor);
        YoYInflationLeg floatingLeg;
        // make the n coupons unless you are making a capLET or floorLET
        Date start = effectiveDate_, end, pay;
        for (Size i = 0; i < tenorN_; i++) {
            if (!asOptionlet_ || (asOptionlet_ && i == tenorN_-1)) {
                start = s[i];
                end = s[i+1];
                pay = end;
                boost::shared_ptr<YoYInflationCoupon> thisCoupon(
                      new YoYInflationCoupon( pay, nominal_, start, end, lag_,
                                              fixingDays_, index_,
                                              1.0, 0.0, // gearing, spread
                                              dc_, bdc_));
                floatingLeg.push_back(thisCoupon);
            }
        }

        boost::shared_ptr<YoYInflationCapFloor> cf(
                          new YoYInflationCapFloor(capFloorType_, floatingLeg,
                                                   capRates, floorRates));

        return cf;
    }


    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::withNominal(Real nominal) {
        nominal_ = nominal;
        return *this;
    }

    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::withTenor(Size n) {
        tenorN_ = n;
        return *this;
    }

    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::withCalendar(const Calendar& cal) {
        cal_ = cal;
        return *this;
    }

    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::withConvention(BusinessDayConvention bdc) {
        bdc_ = bdc;
        return *this;
    }

    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::withDayCount(const DayCounter& dc) {
        dc_ = dc;
        return *this;
    }

    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::asOptionlet() {
        asOptionlet_ = true;
        return *this;
    }

    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::withPricingEngine(
                             const boost::shared_ptr<PricingEngine>& pricer) {
        pricer_ = pricer;
        return *this;
    }

}

