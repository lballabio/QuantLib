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

#include <ql/cashflows/cashflows.hpp>
#include <ql/instruments/inflationcapfloor.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {


    std::ostream& operator<<(std::ostream& out,
                             YoYInflationCapFloor::Type t) {
        switch (t) {
            case YoYInflationCapFloor::Cap:
                return out << "YoYInflationCap";
            case YoYInflationCapFloor::Floor:
                return out << "YoYInflationFloor";
            case YoYInflationCapFloor::Collar:
                return out << "YoYInflationCollar";
            default:
                QL_FAIL("unknown YoYInflationCapFloor::Type (" << Integer(t) << ")");
        }
    }

    YoYInflationCapFloor::YoYInflationCapFloor(YoYInflationCapFloor::Type type,
                                               Leg yoyLeg,
                                               std::vector<Rate> capRates,
                                               std::vector<Rate> floorRates)
    : type_(type), yoyLeg_(std::move(yoyLeg)), capRates_(std::move(capRates)),
      floorRates_(std::move(floorRates)) {
        if (type_ == Cap || type_ == Collar) {
            QL_REQUIRE(!capRates_.empty(), "no cap rates given");
            capRates_.reserve(yoyLeg_.size());
            while (capRates_.size() < yoyLeg_.size())
                capRates_.push_back(capRates_.back());
        }
        if (type_ == Floor || type_ == Collar) {
            QL_REQUIRE(!floorRates_.empty(), "no floor rates given");
            floorRates_.reserve(yoyLeg_.size());
            while (floorRates_.size() < yoyLeg_.size())
                floorRates_.push_back(floorRates_.back());
        }
        Leg::const_iterator i;
        for (i = yoyLeg_.begin(); i != yoyLeg_.end(); ++i)
            registerWith(*i);

        registerWith(Settings::instance().evaluationDate());
    }

    YoYInflationCapFloor::YoYInflationCapFloor(YoYInflationCapFloor::Type type,
                                               Leg yoyLeg,
                                               const std::vector<Rate>& strikes)
    : type_(type), yoyLeg_(std::move(yoyLeg)) {
        QL_REQUIRE(!strikes.empty(), "no strikes given");
        if (type_ == Cap) {
            capRates_ = strikes;
            capRates_.reserve(yoyLeg_.size());
            while (capRates_.size() < yoyLeg_.size())
                capRates_.push_back(capRates_.back());
        } else if (type_ == Floor) {
            floorRates_ = strikes;
            floorRates_.reserve(yoyLeg_.size());
            while (floorRates_.size() < yoyLeg_.size())
                floorRates_.push_back(floorRates_.back());
        } else
            QL_FAIL("only Cap/Floor types allowed in this constructor");

        Leg::const_iterator i;
        for (i = yoyLeg_.begin(); i != yoyLeg_.end(); ++i)
            registerWith(*i);

        registerWith(Settings::instance().evaluationDate());
    }

    bool YoYInflationCapFloor::isExpired() const {
        for (Size i=yoyLeg_.size(); i>0; --i)
            if (!yoyLeg_[i-1]->hasOccurred())
                return false;
        return true;
    }

    Date YoYInflationCapFloor::startDate() const {
        return CashFlows::startDate(yoyLeg_);
    }

    Date YoYInflationCapFloor::maturityDate() const {
        return CashFlows::maturityDate(yoyLeg_);
    }

    ext::shared_ptr<YoYInflationCoupon>
    YoYInflationCapFloor::lastYoYInflationCoupon() const {
        ext::shared_ptr<CashFlow> lastCF(yoyLeg_.back());
        ext::shared_ptr<YoYInflationCoupon> lastYoYInflationCoupon =
        ext::dynamic_pointer_cast<YoYInflationCoupon>(lastCF);
        return lastYoYInflationCoupon;
    }

    ext::shared_ptr<YoYInflationCapFloor> YoYInflationCapFloor::optionlet(const Size i) const {
        QL_REQUIRE(i < yoyLeg().size(),
                   io::ordinal(i+1) << " optionlet does not exist, only " <<
                   yoyLeg().size());
        Leg cf(1, yoyLeg()[i]);

        std::vector<Rate> cap, floor;
        if (type() == Cap || type() == Collar)
            cap.push_back(capRates()[i]);
        if (type() == Floor || type() == Collar)
            floor.push_back(floorRates()[i]);

        return ext::make_shared<YoYInflationCapFloor>(type(),
                                                    cf, cap, floor);
    }

    void YoYInflationCapFloor::setupArguments(PricingEngine::arguments* args) const {
        auto* arguments = dynamic_cast<YoYInflationCapFloor::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        Size n = yoyLeg_.size();

        arguments->startDates.resize(n);
        arguments->fixingDates.resize(n);
        arguments->payDates.resize(n);
        arguments->accrualTimes.resize(n);
        arguments->nominals.resize(n);
        arguments->gearings.resize(n);
        arguments->capRates.resize(n);
        arguments->floorRates.resize(n);
        arguments->spreads.resize(n);

        arguments->type = type_;

        for (Size i=0; i<n; ++i) {
            ext::shared_ptr<YoYInflationCoupon> coupon =
            ext::dynamic_pointer_cast<YoYInflationCoupon>(
                                                            yoyLeg_[i]);
            QL_REQUIRE(coupon, "non-YoYInflationCoupon given");
            arguments->startDates[i] = coupon->accrualStartDate();
            arguments->fixingDates[i] = coupon->fixingDate();
            arguments->payDates[i] = coupon->date();

            // this is passed explicitly for precision
            arguments->accrualTimes[i] = coupon->accrualPeriod();

            arguments->nominals[i] = coupon->nominal();
            Spread spread = coupon->spread();
            Real gearing = coupon->gearing();
            arguments->gearings[i] = gearing;
            arguments->spreads[i] = spread;

            if (type_ == Cap || type_ == Collar)
                arguments->capRates[i] = (capRates_[i]-spread)/gearing;
            else
                arguments->capRates[i] = Null<Rate>();

            if (type_ == Floor || type_ == Collar)
                arguments->floorRates[i] = (floorRates_[i]-spread)/gearing;
            else
                arguments->floorRates[i] = Null<Rate>();
        }
    }

    void YoYInflationCapFloor::arguments::validate() const {
        QL_REQUIRE(payDates.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of pay dates ("
                   << payDates.size() << ")");
        QL_REQUIRE(accrualTimes.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of accrual times ("
                   << accrualTimes.size() << ")");
        QL_REQUIRE(type == YoYInflationCapFloor::Floor ||
                   capRates.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of cap rates ("
                   << capRates.size() << ")");
        QL_REQUIRE(type == YoYInflationCapFloor::Cap ||
                   floorRates.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of floor rates ("
                   << floorRates.size() << ")");
        QL_REQUIRE(gearings.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of gearings ("
                   << gearings.size() << ")");
        QL_REQUIRE(spreads.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of spreads ("
                   << spreads.size() << ")");
        QL_REQUIRE(nominals.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of nominals ("
                   << nominals.size() << ")");
    }

    Rate YoYInflationCapFloor::atmRate(const YieldTermStructure& discountCurve) const {
        return CashFlows::atmRate(yoyLeg_, discountCurve,
                                  false, discountCurve.referenceDate());
    }


}
