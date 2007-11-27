/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Katiuscia Manzoni
 Copyright (C) 2007 Giorgio Facchinetti

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
#include <ql/pricingengines/blackformula.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    OptionletStripper1::OptionletStripper1(
            const boost::shared_ptr<CapFloorTermVolSurface>& termVolSurface,
            const boost::shared_ptr<IborIndex>& index,
            Rate switchStrike,
            Real accuracy)
    : OptionletStripper(termVolSurface, index),
    volQuotes_(nOptionletTenors_,
               std::vector<boost::shared_ptr<SimpleQuote> >(nStrikes_)),
      floatingSwitchStrike_(switchStrike==Null<Rate>() ? true : false),
      capFlooMatrixNotInitialized_(true),
      switchStrike_(switchStrike),
      accuracy_(accuracy){

        capFloorPrices_ = Matrix(nOptionletTenors_, nStrikes_);
        optionletPrices_ = Matrix(nOptionletTenors_, nStrikes_);
        capFloorVols_ = Matrix(nOptionletTenors_, nStrikes_);
        Real firstGuess = 0.14;
        optionletStDevs_ = Matrix(nOptionletTenors_, nStrikes_, firstGuess);

        capFloors_ = CapFloorMatrix(nOptionletTenors_);
    }

    void OptionletStripper1::performCalculations() const {

        // update dates
        const Date& referenceDate = termVolSurface_->referenceDate();
        const DayCounter& dc = termVolSurface_->dayCounter();
        boost::shared_ptr<BlackCapFloorEngine> dummy(new
                      BlackCapFloorEngine(index_->termStructure(), 0.20, dc));
        for (Size i=0; i<nOptionletTenors_; ++i) {
            CapFloor temp = MakeCapFloor(CapFloor::Cap,
                                         capFloorLengths_[i],
                                         index_,
                                         0.04, // dummy strike
                                         0*Days)
                .withPricingEngine(dummy);
            boost::shared_ptr<FloatingRateCoupon> lFRC =
                                                temp.lastFloatingRateCoupon();
            optionletDates_[i] = lFRC->fixingDate();
            optionletPaymentDates_[i] = lFRC->date();
            optionletAccrualPeriods_[i] = lFRC->accrualPeriod();
            optionletTimes_[i] = dc.yearFraction(referenceDate,
                                                 optionletDates_[i]);
        }

        if (floatingSwitchStrike_) { // && capFlooMatrixNotInitialized_)
            Rate averageAtmOptionletRate = 0.0;
            for (Size i=0; i<nOptionletTenors_; ++i) {
                atmOptionletRate_[i] = index_->fixing(optionletDates_[i],true);
                averageAtmOptionletRate += atmOptionletRate_[i];
            }
            switchStrike_ = averageAtmOptionletRate / nOptionletTenors_;
        }

        const std::vector<Rate>& strikes = termVolSurface_->strikes();
        // initialize CapFloorMatrix
        if (capFlooMatrixNotInitialized_) {
            for (Size i=0; i<nOptionletTenors_; ++i)
                capFloors_[i].resize(nStrikes_);
            // construction might go here
            for (Size j=0; j<nStrikes_; ++j) {
                // using out-of-the-money options
                CapFloor::Type capFloorType = strikes[j] < switchStrike_ ?
                                       CapFloor::Floor : CapFloor::Cap;
                for (Size i=0; i<nOptionletTenors_; ++i) {
                    volQuotes_[i][j]= boost::shared_ptr<SimpleQuote>(new SimpleQuote());
                    boost::shared_ptr<BlackCapFloorEngine> engine(new
                        BlackCapFloorEngine(index_->termStructure(),
                                            Handle<Quote>(volQuotes_[i][j]),
                                            dc));
                    capFloors_[i][j] = MakeCapFloor(capFloorType,
                                                    capFloorLengths_[i], index_,
                                                    strikes[j], 0*Days)
                        .withPricingEngine(engine);
                }
            }
            capFlooMatrixNotInitialized_ = false;
        }

        for (Size j=0; j<nStrikes_; ++j) {

            CapFloor::Type capFloorType = strikes[j] < switchStrike_ ?
                                   CapFloor::Floor : CapFloor::Cap;
            Option::Type optionletType = capFloorType==CapFloor::Floor ?
                                   Option::Put : Option::Call;

            Real previousCapFloorPrice = 0.0;
            for (Size i=0; i<nOptionletTenors_; ++i) {

                capFloorVols_[i][j] = termVolSurface_->volatility(
                    capFloorLengths_[i], strikes[j], true);
                volQuotes_[i][j]->setValue(capFloorVols_[i][j]);

                capFloorPrices_[i][j] = capFloors_[i][j]->NPV();
                optionletPrices_[i][j] = capFloorPrices_[i][j] -
                                                        previousCapFloorPrice;
                previousCapFloorPrice = capFloorPrices_[i][j];
                DiscountFactor d = index_->termStructure()->discount(
                                                   optionletPaymentDates_[i]);
                DiscountFactor optionletAnnuity=optionletAccrualPeriods_[i]*d;
                try {
                    optionletStDevs_[i][j] =
                        blackFormulaImpliedStdDev(optionletType,
                                                  strikes[j],
                                                  atmOptionletRate_[i],
                                                  optionletPrices_[i][j],
                                                  optionletAnnuity,
                                                  optionletStDevs_[i][j],
                                                  accuracy_);
                } catch (std::exception& e) {
                    QL_FAIL("could not bootstrap the optionlet:"
                            "\n fixing date:   " << optionletDates_[i] <<
                            "\n payment date:  " << optionletPaymentDates_[i] <<
                            "\n type:          " << optionletType <<
                            "\n strike:        " << io::rate(strikes[j]) <<
                            "\n atm:           " << io::rate(atmOptionletRate_[i]) <<
                            "\n price:         " << optionletPrices_[i][j] <<
                            "\n annuity:       " << optionletAnnuity <<
                            "\n error message: " << e.what());
                }
                optionletVolatilities_[i][j] = optionletStDevs_[i][j] /
                                                std::sqrt(optionletTimes_[i]);
            }
        }

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
