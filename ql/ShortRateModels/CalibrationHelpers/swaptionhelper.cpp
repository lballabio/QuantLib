
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/ShortRateModels/CalibrationHelpers/swaptionhelper.hpp>
#include <ql/PricingEngines/Swaption/blackswaption.hpp>
#include <ql/Instruments/payoffs.hpp>

namespace QuantLib {

    SwaptionHelper::SwaptionHelper(
                         const Period& maturity,
                         const Period& length,
                         const RelinkableHandle<Quote>& volatility,
                         const Handle<Xibor>& index,
                         const RelinkableHandle<TermStructure>& termStructure)
    : CalibrationHelper(volatility,termStructure) {

        Period indexTenor = index->tenor();
        int frequency;
        if (indexTenor.units() == Months) {
            QL_REQUIRE((12%indexTenor.length()) == 0, 
                       "Invalid index tenor");
            frequency = 12/indexTenor.length();
        } else if (indexTenor.units() == Years) {
            QL_REQUIRE(indexTenor.length()==1, "Invalid index tenor");
            frequency=1;
        } else {
            QL_FAIL("index tenor not valid!");
        }
        Date startDate = index->calendar().advance(
                                                   termStructure->referenceDate(),
                                                   maturity.length(), maturity.units());
        Rate fixedRate = 0.04;//dummy value
        swap_ = Handle<SimpleSwap>(new SimpleSwap(
                                                  false,
                                                  startDate,
                                                  length.length(),
                                                  length.units(),
                                                  index->calendar(),
                                                  index->rollingConvention(),
                                                  1.0,
                                                  frequency,
                                                  fixedRate,
                                                  false,
                                                  index->dayCounter(),
                                                  frequency,
                                                  index,
                                                  0,//FIXME
                                                  0.0,
                                                  termStructure));
        Rate fairFixedRate = swap_->fairRate();
        swap_ = Handle<SimpleSwap>(new SimpleSwap(
                            false, startDate, length.length(), length.units(),
                            index->calendar(), index->rollingConvention(),
                            1.0, frequency, fairFixedRate, false,
                            index->dayCounter(), frequency, index,
                            0,//FIXME
                            0.0, termStructure));
        exerciseRate_ = fairFixedRate;
        engine_  = Handle<PricingEngine>(new BlackSwaption(blackModel_));
        Date exerciseDate = index->calendar().roll(
                                       startDate, index->rollingConvention());

        swaption_ = Handle<Swaption>(new Swaption(
            swap_,
            Handle<Exercise>(new EuropeanExercise(exerciseDate)),
            termStructure,
            engine_));
        marketValue_ = blackPrice(volatility_->value());
    }

    void SwaptionHelper::addTimesTo(std::list<Time>& times) const {
        Swaption::arguments* params =
            dynamic_cast<Swaption::arguments*>(engine_->arguments());
        swaption_->setupArguments(params);
        Size i;
        for (i=0; i<params->stoppingTimes.size(); i++)
            times.push_back(params->stoppingTimes[i]);
        for (i=0; i<params->fixedResetTimes.size(); i++)
            times.push_back(params->fixedResetTimes[i]);
        for (i=0; i<params->fixedPayTimes.size(); i++)
            times.push_back(params->fixedPayTimes[i]);
        for (i=0; i<params->floatingResetTimes.size(); i++)
            times.push_back(params->floatingResetTimes[i]);
        for (i=0; i<params->floatingPayTimes.size(); i++)
            times.push_back(params->floatingPayTimes[i]);
    }

    double SwaptionHelper::modelValue() {
        swaption_->setPricingEngine(engine_);
        return swaption_->NPV();
    }

    double SwaptionHelper::blackPrice(double sigma) const {
        Handle<Quote> vol(new SimpleQuote(sigma));
        Handle<BlackModel> blackModel(
                         new BlackModel(RelinkableHandle<Quote>(vol), 
                                        termStructure_));
        Handle<PricingEngine> black(new BlackSwaption(blackModel));
        swaption_->setPricingEngine(black);
        double value = swaption_->NPV();
        swaption_->setPricingEngine(engine_);
        return value;
    }

}
