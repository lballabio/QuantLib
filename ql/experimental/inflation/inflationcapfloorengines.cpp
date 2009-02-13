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

#include <ql/experimental/inflation/inflationcapfloorengines.hpp>
#include <ql/experimental/inflation/capflooredinflationcoupon.hpp>
#include <ql/experimental/inflation/inflationcouponpricer.hpp>
#include <ql/experimental/inflation/yoyoptionletvolatilitystructures.hpp>

namespace QuantLib {

    YoYInflationCapFloorEngine::YoYInflationCapFloorEngine(
                      const Handle<YieldTermStructure>& termStructure,
                      const Handle<YoYOptionletVolatilitySurface>& volatility)
    : termStructure_(termStructure), volatility_(volatility) {
        registerWith(termStructure_);
        registerWith(volatility_);
    }

    YoYInflationCapFloorEngine::YoYInflationCapFloorEngine(
                              const Handle<YieldTermStructure>& termStructure)
    : termStructure_(termStructure) {
        registerWith(termStructure_);
    }


    Handle<YieldTermStructure> YoYInflationCapFloorEngine::termStructure() {
        return termStructure_;
    }


    Handle<YoYOptionletVolatilitySurface>
    YoYInflationCapFloorEngine::volatility() {
        return volatility_;
    }

    void YoYInflationCapFloorEngine::setVolatility(
                             const Handle<YoYOptionletVolatilitySurface> &v) {
        if (!volatility_.empty())
            unregisterWith(volatility_);
        volatility_ = v;
        registerWith(volatility_);
        update();
    }



    BlackYoYInflationCapFloorEngine::BlackYoYInflationCapFloorEngine(
                      const Handle<YieldTermStructure>& termStructure,
                      const Handle<YoYOptionletVolatilitySurface>& volatility)
    : YoYInflationCapFloorEngine(termStructure, volatility) {}


    void BlackYoYInflationCapFloorEngine::calculate() const {

        Real value = 0.0;
        std::vector<Real> optionletsPrice;
        YoYInflationCapFloor::Type type = arguments_.type;

        // now build a capfloored yoy inflation coupon
        // from the inflation coupon you've been given
        // using the strikes in the arguments
        for (Size i=0; i<arguments_.floatingLeg.size(); i++) {
            Rate c = arguments_.capRates[i];
            Rate f = arguments_.floorRates[i];
            // make very sure that the strikes are correct
            if ( type == YoYInflationCapFloor::Cap ) f = Null<Rate>();
            if ( type == YoYInflationCapFloor::Floor ) c = Null<Rate>();
            // get a new capfloored coupon
            CappedFlooredYoYInflationCoupon temp(arguments_.floatingLeg[i],
                                                 c, f, false);
            boost::shared_ptr<BlackYoYInflationCouponPricer>
                yoyPricer(new BlackYoYInflationCouponPricer(volatility_));
            temp.setPricer(yoyPricer);
            Date d = arguments_.floatingLeg[i]->date();
            optionletsPrice.push_back(
                                 termStructure_->discount(d) * temp.amount());
            value += optionletsPrice.back();
        }

        results_.value = value;
        results_.additionalResults["optionletsPrice"] = optionletsPrice;
    }


    UnitDisplacedBlackYoYInflationCapFloorEngine::
    UnitDisplacedBlackYoYInflationCapFloorEngine(
                      const Handle<YieldTermStructure>& termStructure,
                      const Handle<YoYOptionletVolatilitySurface>& volatility)
    : YoYInflationCapFloorEngine(termStructure, volatility) {}


    void UnitDisplacedBlackYoYInflationCapFloorEngine::calculate() const {

        Real value = 0.0;
        std::vector<Real> optionletsPrice;
        YoYInflationCapFloor::Type type = arguments_.type;

        // now build a capfloored yoy inflation coupon
        // from the inflation coupon you've been given
        // using the strikes in the arguments
        for (Size i=0; i<arguments_.floatingLeg.size(); i++) {
            Rate c = arguments_.capRates[i];
            Rate f = arguments_.floorRates[i];
            // make very sure that the strikes are correct
            if ( type == YoYInflationCapFloor::Cap ) f = Null<Rate>();
            if ( type == YoYInflationCapFloor::Floor ) c = Null<Rate>();
            // get a new capfloored coupon
            CappedFlooredYoYInflationCoupon temp(arguments_.floatingLeg[i],
                                                 c, f, false);
            boost::shared_ptr<UnitDisplacedBlackYoYInflationCouponPricer>
                yoyPricer(new UnitDisplacedBlackYoYInflationCouponPricer(
                                                                volatility_));
            temp.setPricer(yoyPricer);
            Date d = arguments_.floatingLeg[i]->date();
            optionletsPrice.push_back(
                                 termStructure_->discount(d) * temp.amount());
            value += optionletsPrice.back();
        }

        results_.value = value;
        results_.additionalResults["optionletsPrice"] = optionletsPrice;
    }


    BachelierYoYInflationCapFloorEngine::BachelierYoYInflationCapFloorEngine(
                      const Handle<YieldTermStructure>& termStructure,
                      const Handle<YoYOptionletVolatilitySurface>& volatility)
    : YoYInflationCapFloorEngine(termStructure, volatility) {}


    void BachelierYoYInflationCapFloorEngine::calculate() const {

        Real value = 0.0;
        std::vector<Real> optionletsPrice;
        YoYInflationCapFloor::Type type = arguments_.type;

        // now build a capfloored yoy inflation coupon
        // from the inflation coupon you've been given
        // using the strikes in the arguments
        for (Size i=0; i<arguments_.floatingLeg.size(); i++) {
            Rate c = arguments_.capRates[i];
            Rate f = arguments_.floorRates[i];
            // make very sure that the strikes are correct
            if ( type == YoYInflationCapFloor::Cap ) f = Null<Rate>();
            if ( type == YoYInflationCapFloor::Floor ) c = Null<Rate>();
            // get a new capfloored coupon
            CappedFlooredYoYInflationCoupon temp(arguments_.floatingLeg[i],
                                                 c, f, false);
            boost::shared_ptr<BachelierYoYInflationCouponPricer>
                yoyPricer(new BachelierYoYInflationCouponPricer(volatility_));
            temp.setPricer(yoyPricer);
            Date d = arguments_.floatingLeg[i]->date();

            optionletsPrice.push_back(
                                 termStructure_->discount(d) * temp.amount());
            value += optionletsPrice.back();
        }

        results_.value = value;
        results_.additionalResults["optionletsPrice"] = optionletsPrice;
    }

}

