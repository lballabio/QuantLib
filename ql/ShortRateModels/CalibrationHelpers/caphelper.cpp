
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

#include <ql/ShortRateModels/CalibrationHelpers/caphelper.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/Instruments/swap.hpp>
#include <ql/PricingEngines/CapFloor/blackcapfloor.hpp>

namespace QuantLib {

    CapHelper::CapHelper(const Period& length,
                         const RelinkableHandle<Quote>& volatility,
                         const boost::shared_ptr<Xibor>& index,
                         const RelinkableHandle<TermStructure>& termStructure)
    : CalibrationHelper(volatility,termStructure) {

        Period indexTenor = index->tenor();
        int frequency;
        if (indexTenor.units() == Months) {
            QL_REQUIRE((12%indexTenor.length()) == 0, 
                       "invalid index tenor");
            frequency = 12/indexTenor.length();
        } else if (indexTenor.units() == Years) {
            QL_REQUIRE(indexTenor.length()==1, "invalid index tenor");
            frequency=1;
        } else {
            QL_FAIL("invalid index tenor");
        }
        Rate fixedRate = 0.04;//dummy value
        Date startDate = termStructure->referenceDate().
            plus(indexTenor.length(), indexTenor.units());
        Date maturity = termStructure->referenceDate().
            plus(length.length(), length.units());

        boost::shared_ptr<Xibor> dummyIndex(
                                        new Xibor("dummy",
                                                  indexTenor.length(),
                                                  indexTenor.units(),
                                                  index->settlementDays(),
                                                  index->currency(),
                                                  index->calendar(),
                                                  index->isAdjusted(),
                                                  index->rollingConvention(),
                                                  termStructure->dayCounter(),
                                                  termStructure));

        std::vector<double> nominals(1,1.0);
        Schedule floatSchedule(index->calendar(), startDate, maturity,
                               frequency, index->rollingConvention(),
                               true);
        std::vector<boost::shared_ptr<CashFlow> > floatingLeg = 
            FloatingRateCouponVector(floatSchedule, nominals, 
                                     index, 0);

        Schedule fixedSchedule(index->calendar(), startDate, maturity,
                               frequency, index->rollingConvention(),
                               false);
        std::vector<boost::shared_ptr<CashFlow> > fixedLeg = 
            FixedRateCouponVector(fixedSchedule, nominals, 
                                  std::vector<Rate>(1, fixedRate), 
                                  index->dayCounter());

        boost::shared_ptr<Swap> swap(
                              new Swap(floatingLeg, fixedLeg, termStructure));
        Rate fairRate = fixedRate - 
            swap->NPV()/swap->secondLegBPS();
        engine_  = boost::shared_ptr<PricingEngine>();
        cap_ = boost::shared_ptr<Cap>(new Cap(floatingLeg, 
                                              std::vector<Rate>(1, fairRate), 
                                              termStructure, engine_));
        marketValue_ = blackPrice(volatility_->value());
    }

    void CapHelper::addTimesTo(std::list<Time>& times) const {
        CapFloor::arguments params;
        cap_->setupArguments(&params);
        Size nPeriods = params.startTimes.size();
        for (Size i=0; i<nPeriods; i++) {
            times.push_back(params.startTimes[i]);
            times.push_back(params.endTimes[i]);
        }
    }

    double CapHelper::modelValue() const {
        cap_->setPricingEngine(engine_);
        return cap_->NPV();
    }

    double CapHelper::blackPrice(double sigma) const {
        boost::shared_ptr<Quote> vol(new SimpleQuote(sigma));
        boost::shared_ptr<BlackModel> blackModel(
                         new BlackModel(RelinkableHandle<Quote>(vol), 
                                        termStructure_));
        boost::shared_ptr<PricingEngine> black(new BlackCapFloor(blackModel));
        cap_->setPricingEngine(black);
        double value = cap_->NPV();
        cap_->setPricingEngine(engine_);
        return value;
    }

}
