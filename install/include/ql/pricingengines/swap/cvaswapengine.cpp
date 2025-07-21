/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Jose Aparicio

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

#include <ql/pricingengines/swap/cvaswapengine.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/exercise.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <ql/time/calendars/nullcalendar.hpp>

namespace QuantLib {
  
  CounterpartyAdjSwapEngine::CounterpartyAdjSwapEngine(
      const Handle<YieldTermStructure>& discountCurve,
      const Handle<PricingEngine>& swaptionEngine,
      const Handle<DefaultProbabilityTermStructure>& ctptyDTS,
      Real ctptyRecoveryRate,
      const Handle<DefaultProbabilityTermStructure>& invstDTS,
      Real invstRecoveryRate)
  : baseSwapEngine_(Handle<PricingEngine>(
      ext::make_shared<DiscountingSwapEngine>(discountCurve))),
    swaptionletEngine_(swaptionEngine),
    discountCurve_(discountCurve),
    defaultTS_(ctptyDTS), 
    ctptyRecoveryRate_(ctptyRecoveryRate),
    invstDTS_(invstDTS.empty() ? Handle<DefaultProbabilityTermStructure>(
        ext::make_shared<FlatHazardRate>(0, NullCalendar(), 1.e-12, 
        ctptyDTS->dayCounter()) ) : invstDTS ),
    invstRecoveryRate_(invstRecoveryRate)
  {
      registerWith(discountCurve);
      registerWith(ctptyDTS);
      registerWith(invstDTS_);
      registerWith(swaptionEngine);
  }

    CounterpartyAdjSwapEngine::CounterpartyAdjSwapEngine(
        const Handle<YieldTermStructure>& discountCurve,
        const Volatility blackVol,
        const Handle<DefaultProbabilityTermStructure>& ctptyDTS,
        Real ctptyRecoveryRate,
        const Handle<DefaultProbabilityTermStructure>& invstDTS,
        Real invstRecoveryRate)
  : baseSwapEngine_(Handle<PricingEngine>(
      ext::make_shared<DiscountingSwapEngine>(discountCurve))),
    swaptionletEngine_(Handle<PricingEngine>(
      ext::make_shared<BlackSwaptionEngine>(discountCurve,
        blackVol))),
    discountCurve_(discountCurve),
    defaultTS_(ctptyDTS), 
    ctptyRecoveryRate_(ctptyRecoveryRate),
    invstDTS_(invstDTS.empty() ? Handle<DefaultProbabilityTermStructure>(
        ext::make_shared<FlatHazardRate>(0, NullCalendar(), 1.e-12, 
        ctptyDTS->dayCounter()) ) : invstDTS ),
    invstRecoveryRate_(invstRecoveryRate)
  {
      registerWith(discountCurve);
      registerWith(ctptyDTS);
      registerWith(invstDTS_);
  }

  CounterpartyAdjSwapEngine::CounterpartyAdjSwapEngine(
        const Handle<YieldTermStructure>& discountCurve,
        const Handle<Quote>& blackVol,
        const Handle<DefaultProbabilityTermStructure>& ctptyDTS,
        Real ctptyRecoveryRate,
        const Handle<DefaultProbabilityTermStructure>& invstDTS,
        Real invstRecoveryRate)
  : baseSwapEngine_(Handle<PricingEngine>(
      ext::make_shared<DiscountingSwapEngine>(discountCurve))),
    swaptionletEngine_(Handle<PricingEngine>(
      ext::make_shared<BlackSwaptionEngine>(discountCurve,
        blackVol))),
    discountCurve_(discountCurve),
    defaultTS_(ctptyDTS), 
    ctptyRecoveryRate_(ctptyRecoveryRate),
    invstDTS_(invstDTS.empty() ? Handle<DefaultProbabilityTermStructure>(
        ext::make_shared<FlatHazardRate>(0, NullCalendar(), 1.e-12, 
        ctptyDTS->dayCounter()) ) : invstDTS ),
    invstRecoveryRate_(invstRecoveryRate)
  {
      registerWith(discountCurve);
      registerWith(ctptyDTS);
      registerWith(invstDTS_);
      registerWith(blackVol);
  }

  void CounterpartyAdjSwapEngine::calculate() const {
      /* both DTS, YTS ref dates and pricing date consistency 
         checks? settlement... */
    QL_REQUIRE(!discountCurve_.empty(),
                 "no discount term structure set");
    QL_REQUIRE(!defaultTS_.empty(),
                 "no ctpty default term structure set");
    QL_REQUIRE(!swaptionletEngine_.empty(),
                 "no swap option engine set");

    QL_REQUIRE(arguments_.nominal != Null<Real>(),
               "non-constant nominals are not supported yet");

    Date priceDate = defaultTS_->referenceDate();

    Real cumOptVal = 0., 
        cumPutVal = 0.;
    // Vanilla swap so 0 leg is floater

    auto nextFD = 
      arguments_.fixedPayDates.begin();
    Date swapletStart = priceDate;
    while (*nextFD < priceDate) ++nextFD;

    // Compute fair spread for strike value:
    // copy args into the non risky engine
    auto* noCVAArgs = dynamic_cast<Swap::arguments*>(baseSwapEngine_->getArguments());
    QL_REQUIRE(noCVAArgs != nullptr, "wrong argument type");

    noCVAArgs->legs = this->arguments_.legs;
    noCVAArgs->payer = this->arguments_.payer;

    baseSwapEngine_->calculate();

    ext::shared_ptr<FixedRateCoupon> coupon = ext::dynamic_pointer_cast<FixedRateCoupon>(arguments_.legs[0][0]);
    QL_REQUIRE(coupon,"dynamic cast of fixed leg coupon failed.");
    Rate baseSwapRate = coupon->rate();

    const auto* vSResults = dynamic_cast<const Swap::results*>(baseSwapEngine_->getResults());
    QL_REQUIRE(vSResults != nullptr, "wrong result type");

    Rate baseSwapFairRate = -baseSwapRate * vSResults->legNPV[1] / 
        vSResults->legNPV[0];
    Real baseSwapNPV = vSResults->value;

    Swap::Type reversedType = arguments_.type == Swap::Payer ? Swap::Receiver : Swap::Payer;

    // Swaplet options summatory:
    while(nextFD != arguments_.fixedPayDates.end()) {
      // iFD coupon not fixed, create swaptionlet:
      ext::shared_ptr<FloatingRateCoupon> floatCoupon = ext::dynamic_pointer_cast<FloatingRateCoupon>(arguments_.legs[1][0]);
      QL_REQUIRE(floatCoupon,"dynamic cast of floating leg coupon failed.");
      ext::shared_ptr<IborIndex> swapIndex = ext::dynamic_pointer_cast<IborIndex>(floatCoupon->index());
      QL_REQUIRE(swapIndex,"dynamic cast of floating leg index failed.");

      // Alternatively one could cap this period to, say, 1M 
      // Period swapPeriod = ext::dynamic_pointer_cast<FloatingRateCoupon>(
      //   arguments_.legs[1][0])->index()->tenor();

      Period baseSwapsTenor(arguments_.fixedPayDates.back().serialNumber() 
	    - swapletStart.serialNumber(), Days);
      ext::shared_ptr<VanillaSwap> swaplet = MakeVanillaSwap(
        baseSwapsTenor,
        swapIndex, 
        baseSwapFairRate // strike
        )
	    .withType(arguments_.type)
	    .withNominal(arguments_.nominal)
          ////////	    .withSettlementDays(2)
        .withEffectiveDate(swapletStart)
        .withTerminationDate(arguments_.fixedPayDates.back());
      ext::shared_ptr<VanillaSwap> revSwaplet = MakeVanillaSwap(
        baseSwapsTenor,
        swapIndex, 
        baseSwapFairRate // strike
        )
	    .withType(reversedType)
	    .withNominal(arguments_.nominal)
          /////////	    .withSettlementDays(2)
        .withEffectiveDate(swapletStart)
        .withTerminationDate(arguments_.fixedPayDates.back());

      Swaption swaptionlet(swaplet, 
        ext::make_shared<EuropeanExercise>(swapletStart));
      Swaption putSwaplet(revSwaplet, 
        ext::make_shared<EuropeanExercise>(swapletStart));
      swaptionlet.setPricingEngine(swaptionletEngine_.currentLink());
      putSwaplet.setPricingEngine(swaptionletEngine_.currentLink());

      // atm underlying swap means that the value of put = value
      // call so this double pricing is not needed
      cumOptVal += swaptionlet.NPV() * defaultTS_->defaultProbability(
          swapletStart, *nextFD);
      cumPutVal += putSwaplet.NPV()  * invstDTS_->defaultProbability(
	      swapletStart, *nextFD);

      swapletStart = *nextFD;
      ++nextFD;
    }
  
    results_.value = baseSwapNPV - (1.-ctptyRecoveryRate_) * cumOptVal
        + (1.-invstRecoveryRate_) * cumPutVal;

    results_.fairRate =  -baseSwapRate * (vSResults->legNPV[1] 
        - (1.-ctptyRecoveryRate_) * cumOptVal + 
          (1.-invstRecoveryRate_) * cumPutVal )
      / vSResults->legNPV[0];

  }


}
