
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file binarybarrieroption.cpp
    \brief Binary Barrier option on a single asset
*/

#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Instruments/binarybarrieroption.hpp>
#include <ql/PricingEngines/Barrier/binarybarrierengines.hpp>

namespace QuantLib {

    BinaryBarrierOption::BinaryBarrierOption(
                         BinaryBarrier::Type binaryBarrierType,
                         double barrier,
                         double cashPayoff,
                         Option::Type type,
                         const RelinkableHandle<Quote>& underlying,
                         const RelinkableHandle<TermStructure>& dividendTS,
                         const RelinkableHandle<TermStructure>& riskFreeTS,
                         const Handle<Exercise>& exercise,
                         const RelinkableHandle<BlackVolTermStructure>& volTS,
                         const Handle<PricingEngine>& engine,
                         const std::string& isinCode, 
                         const std::string& description)
    : OneAssetOption(underlying, dividendTS, riskFreeTS,
      exercise, volTS, engine, isinCode, description),
      binaryBarrierType_(binaryBarrierType), barrier_(barrier), cashPayoff_(cashPayoff),
      type_(type) {

        if (IsNull(engine)) {
            switch (exercise->type()) {
              case Exercise::European:
                setPricingEngine(Handle<PricingEngine>(
                                           new AnalyticEuropeanBinaryBarrierEngine));
                break;
              case Exercise::American:
                setPricingEngine(Handle<PricingEngine>(
                                           new AnalyticAmericanBinaryBarrierEngine));
                break;
            }
        }

    }

    bool BinaryBarrierOption::isExpired() const {
        return exercise_->lastDate() < riskFreeTS_->referenceDate();
    }

    void BinaryBarrierOption::setupArguments(Arguments* args) const {

        BinaryBarrierOption::arguments* moreArgs =
            dynamic_cast<BinaryBarrierOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0,
                   "BinaryBarrierOption::setupArguments : "
                   "Arguments wrong type");

        moreArgs->payoff = Handle<PlainVanillaPayoff>(
                                      new PlainVanillaPayoff(type_,barrier_));

        moreArgs->binaryBarrierType = binaryBarrierType_;
        moreArgs->barrier = barrier_;
        moreArgs->cashPayoff = cashPayoff_;

        OneAssetOption::arguments* arguments =
            dynamic_cast<OneAssetOption::arguments*>(args);
        QL_REQUIRE(arguments != 0,
                   "BinaryBarrierOption::setupArguments : "
                   "wrong argument type");
        OneAssetOption::setupArguments(arguments);
    
    }

    void BinaryBarrierOption::performCalculations() const {
        // enforce in this class any check on engine/payoff
        OneAssetOption::performCalculations();
    }

    void BinaryBarrierOption::arguments::validate() const {
        #if defined(QL_PATCH_MICROSOFT)
        OneAssetOption::arguments copy = *this;
        copy.validate();
        #else
        OneAssetOption::arguments::validate();
        #endif
        QL_REQUIRE(barrier != Null<double>(),
                   "BinaryBarrierOption: no barrier given");
        QL_REQUIRE(cashPayoff != Null<double>(),
                   "BinaryBarrierOption: no cash payoff given");
    }


}

