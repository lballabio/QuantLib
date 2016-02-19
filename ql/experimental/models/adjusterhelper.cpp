/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include <ql/experimental/models/adjusterhelper.hpp>
#include <ql/pricingengines/swaption/gaussian1dfloatfloatswaptionengine.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>

#include <boost/assign/list_of.hpp>
#include <boost/make_shared.hpp>

namespace QuantLib {

void AdjusterHelper::performCalculations() const {
    // set up exotic coupon as float float swap and option on it
    Date start = index_->valueDate(fixingDate_);
    Date end = paymentDate_;
    Schedule sched(start, end, (end - start) * Days, NullCalendar(), Unadjusted,
                   Unadjusted, DateGeneration::Forward, false);
    instrument_ = boost::shared_ptr<FloatFloatSwap>(new FloatFloatSwap(
        VanillaSwap::Receiver, 100000000.0, 0.0, sched, index_,
        Actual365Fixed(), sched, index_, Actual365Fixed(), false, false, 1.0,
        0.0, cappedRate_, flooredRate_));
    boost::shared_ptr<Exercise> dummyExercise =
        boost::make_shared<EuropeanExercise>(fixingDate_);
    dummyOption_ =
        boost::make_shared<FloatFloatSwaption>(instrument_, dummyExercise);

    // use the same discounting curve for the reference price as in the model
    boost::shared_ptr<Gaussian1dFloatFloatSwaptionEngine> tmpEngine =
        boost::dynamic_pointer_cast<Gaussian1dFloatFloatSwaptionEngine>(
            modelEngine_);
    QL_REQUIRE(tmpEngine != NULL,
               "engine must be a Gaussian1dFloatFloatSwaptionEngine");
    Handle<YieldTermStructure> yts_discount = tmpEngine->discountingCurve();

    // calculate reference price
    Leg exotic = instrument_->leg(0);
    QuantLib::setCouponPricer(exotic, referencePricer_);
    referenceValue_ = CashFlows::npv(exotic, **yts_discount, false);
}

Real AdjusterHelper::modelValue() const {
    calculate();
    dummyOption_->setPricingEngine(modelEngine_);
    return dummyOption_->result<Real>("underlyingValue");
}
} // namespace QuantLib
