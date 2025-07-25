/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Katiuscia Manzoni
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2015 Michael von den Driesch
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

#include <ql/termstructures/volatility/optionlet/optionletstripper1.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/pricingengine.hpp>
#include <ql/pricingengines/capfloor/bacheliercapfloorengine.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    OptionletStripper1::OptionletStripper1(
        const ext::shared_ptr<CapFloorTermVolSurface>& termVolSurface,
        const ext::shared_ptr<IborIndex>& index,
        Rate switchStrike,
        Real accuracy,
        Natural maxIter,
        const Handle<YieldTermStructure>& discount,
        const VolatilityType type,
        const Real displacement,
        bool dontThrow,
        ext::optional<Period> optionletFrequency)
    : OptionletStripper(termVolSurface, index, discount, type, displacement, optionletFrequency),
      floatingSwitchStrike_(switchStrike == Null<Rate>()), switchStrike_(switchStrike),
      accuracy_(accuracy), maxIter_(maxIter), dontThrow_(dontThrow) {

        capFloorPrices_ = Matrix(nOptionletTenors_, nStrikes_);
        optionletPrices_ = Matrix(nOptionletTenors_, nStrikes_);
        capletVols_ = Matrix(nOptionletTenors_, nStrikes_);
        capFloorVols_ = Matrix(nOptionletTenors_, nStrikes_);

        Real firstGuess = 0.14; // guess is only used for shifted lognormal vols
        optionletStDevs_ = Matrix(nOptionletTenors_, nStrikes_, firstGuess);
    }

    void OptionletStripper1::performCalculations() const {

        // update dates
        const Date& referenceDate = termVolSurface_->referenceDate();
        const DayCounter& dc = termVolSurface_->dayCounter();
        ext::shared_ptr<BlackCapFloorEngine> dummy(new
                    BlackCapFloorEngine(// discounting does not matter here
                                        iborIndex_->forwardingTermStructure(),
                                        0.20, dc));
        for (Size i=0; i<nOptionletTenors_; ++i) {
            CapFloor temp = MakeCapFloor(CapFloor::Cap,
                                         capFloorLengths_[i],
                                         iborIndex_,
                                         0.04, // dummy strike
                                         0*Days)
                .withPricingEngine(dummy);
            ext::shared_ptr<FloatingRateCoupon> lFRC =
                                                temp.lastFloatingRateCoupon();
            optionletDates_[i] = lFRC->fixingDate();
            optionletPaymentDates_[i] = lFRC->date();
            optionletAccrualPeriods_[i] = lFRC->accrualPeriod();
            optionletTimes_[i] = dc.yearFraction(referenceDate,
                                                 optionletDates_[i]);
            atmOptionletRate_[i] = lFRC->indexFixing();
        }

        if (floatingSwitchStrike_) {
            Rate averageAtmOptionletRate = 0.0;
            for (Size i=0; i<nOptionletTenors_; ++i) {
                averageAtmOptionletRate += atmOptionletRate_[i];
            }
            switchStrike_ = averageAtmOptionletRate / nOptionletTenors_;
        }

        const Handle<YieldTermStructure>& discountCurve =
            discount_.empty() ?
                iborIndex_->forwardingTermStructure() :
                discount_;

        const std::vector<Rate>& strikes = termVolSurface_->strikes();

        ext::shared_ptr<PricingEngine> capFloorEngine;
        ext::shared_ptr<SimpleQuote> volQuote(new SimpleQuote);

        if (volatilityType_ == ShiftedLognormal) {
            capFloorEngine = ext::make_shared<BlackCapFloorEngine>(
                        
                            discountCurve, Handle<Quote>(volQuote),
                            dc, displacement_);
        } else if (volatilityType_ == Normal) {
            capFloorEngine = ext::make_shared<BachelierCapFloorEngine>(
                        
                            discountCurve, Handle<Quote>(volQuote),
                            dc);
        } else {
            QL_FAIL("unknown volatility type: " << volatilityType_);
        }

        for (Size j=0; j<nStrikes_; ++j) {
            // using out-of-the-money options
            CapFloor::Type capFloorType =
                strikes[j] < switchStrike_ ? CapFloor::Floor : CapFloor::Cap;
            Option::Type optionletType =
                strikes[j] < switchStrike_ ? Option::Put : Option::Call;

            Real previousCapFloorPrice = 0.0;
            for (Size i=0; i<nOptionletTenors_; ++i) {

                capFloorVols_[i][j] = termVolSurface_->volatility(
                    capFloorLengths_[i], strikes[j], true);
                volQuote->setValue(capFloorVols_[i][j]);
                ext::shared_ptr<CapFloor> capFloor =
                    MakeCapFloor(capFloorType, capFloorLengths_[i],
                                 iborIndex_, strikes[j], -0 * Days)
                        .withPricingEngine(capFloorEngine);
                capFloorPrices_[i][j] = capFloor->NPV();
                optionletPrices_[i][j] = capFloorPrices_[i][j] -
                                                        previousCapFloorPrice;
                previousCapFloorPrice = capFloorPrices_[i][j];
                DiscountFactor d =
                    discountCurve->discount(optionletPaymentDates_[i]);
                DiscountFactor optionletAnnuity=optionletAccrualPeriods_[i]*d;
                try {
                  if (volatilityType_ == ShiftedLognormal) {
                    optionletStDevs_[i][j] = blackFormulaImpliedStdDev(
                        optionletType, strikes[j], atmOptionletRate_[i],
                        optionletPrices_[i][j], optionletAnnuity, displacement_,
                        optionletStDevs_[i][j], accuracy_, maxIter_);
                  } else if (volatilityType_ == Normal) {
                    optionletStDevs_[i][j] =
                        std::sqrt(optionletTimes_[i]) *
                        bachelierBlackFormulaImpliedVol(
                            optionletType, strikes[j], atmOptionletRate_[i],
                            optionletTimes_[i], optionletPrices_[i][j],
                            optionletAnnuity);
                  } else {
                    QL_FAIL("Unknown volatility type: " << volatilityType_);
                  }
                }
                catch (std::exception &e) {
                    if(dontThrow_)
                        optionletStDevs_[i][j]=0.0;
                    else
                        QL_FAIL("could not bootstrap optionlet:"
                            "\n type:    " << optionletType <<
                            "\n strike:  " << io::rate(strikes[j]) <<
                            "\n atm:     " << io::rate(atmOptionletRate_[i]) <<
                            "\n price:   " << optionletPrices_[i][j] <<
                            "\n annuity: " << optionletAnnuity <<
                            "\n expiry:  " << optionletDates_[i] <<
                            "\n error:   " << e.what());
                }
                optionletVolatilities_[i][j] = optionletStDevs_[i][j] /
                                                std::sqrt(optionletTimes_[i]);
            }
        }

    }

    const Matrix &OptionletStripper1::capletVols() const {
        calculate();
        return capletVols_;
    }

    const Matrix& OptionletStripper1::capFloorPrices() const {
        calculate();
        return capFloorPrices_;
    }

    const Matrix& OptionletStripper1::capFloorVolatilities() const {
        calculate();
        return capFloorVols_;
    }

    const Matrix& OptionletStripper1::optionletPrices() const {
        calculate();
        return optionletPrices_;
    }

    Rate OptionletStripper1::switchStrike() const {
        if (floatingSwitchStrike_)
            calculate();
        return switchStrike_;
    }

}
