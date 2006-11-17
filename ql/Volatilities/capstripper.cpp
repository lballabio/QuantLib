/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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

/*! \file capletvariancecurve.hpp
    \brief caplet variance curve
*/

#include <ql/Volatilities/capstripper.hpp>
#include <ql/Patterns/lazyobject.hpp>
#include <ql/capvolstructures.hpp>
#include <ql/types.hpp>
#include <ql/calendar.hpp>
#include <ql/daycounter.hpp>
#include <ql/quote.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/analysis.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/PricingEngines/CapFloor/blackcapfloorengine.hpp>
#include <ql/Indexes/xibor.hpp>
#include <ql/schedule.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Utilities/dataformatters.hpp>
#include <ql/Volatilities/capletvolatilitiesstructures.hpp>


namespace QuantLib {

    class ImpliedVolHelper{
    public:
        ImpliedVolHelper(boost::shared_ptr<CapFloor> cap,
                         Real targetValue,
                         Real& volatilityParameter):
                         targetValue_(targetValue), cap_(cap),
                         volatilityParameter_(volatilityParameter){};

        Real operator()(Real x) const {
            volatilityParameter_ = x;
            cap_->update();
            return cap_->NPV() - targetValue_;
        };
    private:
        Real targetValue_;
        boost::shared_ptr<CapFloor> cap_;
        Real& volatilityParameter_;
    };
    
    void fitVolatilityParameter(boost::shared_ptr<CapFloor> mkData,
                                    Real& volatilityParameter,
                                    Real targetValue,
                                    Real accuracy = 1e-5,
                                    Size maxEvaluations = 1000,
                                    Volatility minVol = 1e-4,
                                    Volatility maxVol = 4) {
        ImpliedVolHelper f(mkData, targetValue, volatilityParameter);
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        volatilityParameter = solver.solve(f, accuracy, volatilityParameter, minVol, maxVol);
    };

    FloatingLeg LegHelper::makeLeg(const Period & startPeriod,
                                    const Period & endPeriod){
        Date startDate = referenceDate_ + startPeriod;
        Date endDate = referenceDate_ + endPeriod;
        Schedule schedule(startDate, endDate, index_->tenor(), calendar_,
                          convention_, convention_, true, false);
        return FloatingRateCouponVector(schedule,
                                        convention_,
                                        std::vector<Real>(1,1),
                                        fixingDays_, index_,
                                        std::vector<Real>(),
                                        std::vector<Spread>(),
                                        index_->dayCounter());
        }

   
    CapsStripper::CapsStripper(
        const Calendar & calendar,
        BusinessDayConvention convention,
        Integer fixingDays,
        const std::vector<Period>& tenors,
        const std::vector<Rate>& strikes,
        const std::vector<std::vector<Handle<Quote> > >& vols,
        const DayCounter& volatilityDayCounter,
        const boost::shared_ptr<Xibor>& index,
        const Handle< YieldTermStructure > termStructure,
        Real impliedVolatilityAccuracy,
        Size maxEvaluations,
        const boost::shared_ptr<SmileSectionsVolStructure> 
            smileSectionsVolStructure)
    : CapletVolatilityStructure(0, calendar),
      volatilityDayCounter_(volatilityDayCounter),
      tenors_(tenors), strikes_(strikes),
      impliedVolatilityAccuracy_(impliedVolatilityAccuracy),
      maxEvaluations_(maxEvaluations){

        QL_REQUIRE(vols.size()==tenors.size(),
                   "mismatch between tenors(" << tenors.size() <<
                   ") and vol rows(" << vols.size() << ")");
        QL_REQUIRE(vols[0].size()==strikes.size(),
                   "mismatch between strikes(" << strikes.size() <<
                   ") and vol columns(" << vols[0].size() << ")");

        LegHelper legHelper(referenceDate(), calendar, fixingDays,
            convention, index);
        marketDataCap_.resize(tenors.size());
        std::vector<FloatingLeg> floatingLegs(tenors_.size());
        for (Size i = 0 ; i < tenors_.size(); i++) {
            floatingLegs[i] = legHelper.makeLeg(index->tenor(),
               tenors[i]);
            Rate atmRate = Cashflows::atmRate(floatingLegs[i], termStructure);
            marketDataCap_[i].resize(strikes_.size());

           for (Size j = 0 ; j < strikes_.size(); j++) {
               boost::shared_ptr<PricingEngine> blackCapFloorEngine(new
                   BlackCapFloorEngine(vols[i][j], volatilityDayCounter));
               if (strikes_[j] < atmRate)
                   marketDataCap_[i][j] = boost::shared_ptr<CapFloor>(new
                       Floor(floatingLegs[i], std::vector<Real>(1,strikes_[j]),
                           termStructure, blackCapFloorEngine));
               else
                    marketDataCap_[i][j] = boost::shared_ptr<CapFloor>(new
                       Cap(floatingLegs[i], std::vector<Real>(1,strikes_[j]),
                           termStructure, blackCapFloorEngine));
               registerWith(marketDataCap_[i][j]);
           }
        }
        // to be changed ...
        if (smileSectionsVolStructure.px== 0)
            parametrizedCapletVolStructure_ 
               = boost::shared_ptr<ParametrizedCapletVolStructure>(
                new BilinInterpCapletVolStructure(referenceDate(),
                                                  volatilityDayCounter,
                                                  marketDataCap_,
                                                  strikes));
        else{
             parametrizedCapletVolStructure_ 
               = boost::shared_ptr<ParametrizedCapletVolStructure>(
                new HybridCapletVolatilityStructure(referenceDate(),
                                                  volatilityDayCounter,
                                                  marketDataCap_,
                                                  strikes,
                                                  smileSectionsVolStructure));
             registerWith(parametrizedCapletVolStructure_);
            }

       Handle<CapletVolatilityStructure> bilinInterpCapletVolStructureHandle(
           parametrizedCapletVolStructure_);
        boost::shared_ptr<PricingEngine> calibBlackCapFloorEngine(new
            BlackCapFloorEngine(bilinInterpCapletVolStructureHandle));
        calibCap_.resize(tenors_.size());
        for (Size i = 0 ; i < tenors_.size(); i++) {
            calibCap_[i].resize(strikes_.size());
            for (Size j = 0 ; j < strikes_.size(); j++) {
                calibCap_[i][j] = boost::shared_ptr<CapFloor>(new
                       CapFloor(*marketDataCap_[i][j]));
                calibCap_[i][j]->setPricingEngine(calibBlackCapFloorEngine);
            }
        } 
    }


    void CapsStripper::performCalculations () const {
        Matrix& volatilityParameters = 
            parametrizedCapletVolStructure_->volatilityParameters();

        for (Size j = 0 ; j < strikes_.size(); j++){
            for (Size i = 0 ; i < tenors_.size(); i++) {
                CapFloor & mktCap = *marketDataCap_[i][j];
                Real capPrice = mktCap.NPV();
                fitVolatilityParameter(calibCap_[i][j], 
                    volatilityParameters[i][j],
                    capPrice, impliedVolatilityAccuracy_, maxEvaluations_);
            }
        }
    }

    Size locateTime(Time x,
                     const std::vector<Time>& values){
        if (x <= values[0])
            return 0;
        if (x >= values.back())
            return values.size()-1;
        Size i = 0;
        while (x > values[i])
            i++;
        return i;
    }

    Volatility CapsStripper::volatilityImpl(Time t, Rate r) const {
            calculate();
            return parametrizedCapletVolStructure_->volatility(t, r, true);
    }
}
