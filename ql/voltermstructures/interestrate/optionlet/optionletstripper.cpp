/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Katiuscia Manzoni

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

#include <ql/voltermstructures/interestrate/optionlet/optionletstripper.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    OptionletStripper::OptionletStripper(
                    const boost::shared_ptr<CapFloorTermVolSurface>& surface,
                    const boost::shared_ptr<IborIndex>& index,
                    Rate switchStrike)
    : surface_(surface), index_(index), 
      nStrikes_(surface->strikes().size()), switchStrike_(switchStrike)
    {
        registerWith(surface);
        registerWith(index);
        registerWith(Settings::instance().evaluationDate());

        Period indexTenor = index->tenor();
        Period maxCapFloorTenor = surface->optionTenors().back();

        // optionlet tenors and capFloor lengths
        optionletTenors_.push_back(indexTenor);
        capFloorLengths_.push_back(optionletTenors_.back()+indexTenor);
        QL_REQUIRE(maxCapFloorTenor>=capFloorLengths_.back(),
                   "too short (" << maxCapFloorTenor <<
                   ") capfloor term vol surface");
        Period nextCapFloorLength = capFloorLengths_.back()+indexTenor;
        while (nextCapFloorLength<=maxCapFloorTenor) {
            optionletTenors_.push_back(capFloorLengths_.back());
            capFloorLengths_.push_back(nextCapFloorLength);
            nextCapFloorLength += indexTenor;
        }
        nOptionletTenors_ = optionletTenors_.size();

        capFloorPrices_ = Matrix(nOptionletTenors_, nStrikes_);
        optionletPrices_ = Matrix(nOptionletTenors_, nStrikes_);
        capFloorVols_ = Matrix(nOptionletTenors_, nStrikes_);
        optionletVols_ = Matrix(nOptionletTenors_, nStrikes_);
        Real firstGuess = 0.14;
        optionletStDevs_ = Matrix(nOptionletTenors_, nStrikes_, firstGuess);
        atmOptionletRate_ = std::vector<Rate>(nOptionletTenors_);
        optionletDates_ = std::vector<Date>(nOptionletTenors_);
        optionletPaymentDates_ = std::vector<Date>(nOptionletTenors_);
        optionletTimes_ = std::vector<Time>(nOptionletTenors_);
        optionletAccrualPeriods_ = std::vector<Time>(nOptionletTenors_);
        capFloors_ = CapFloorMatrix(nOptionletTenors_);

        if (switchStrike_==Null<Rate>())
            switchStrike_ = 0.04;
    }

    void OptionletStripper::performCalculations() const {

        const Date& referenceDate = surface_->referenceDate();
        const std::vector<Rate>& strikes = surface_->strikes();
        const Calendar& cal = index_->fixingCalendar();
        const DayCounter& dc = surface_->dayCounter();
        for (Size i=0; i<nOptionletTenors_; ++i) {
            boost::shared_ptr<BlackCapFloorEngine> dummy(new
                                         BlackCapFloorEngine(0.20, dc));
            CapFloor temp = MakeCapFloor(CapFloor::Cap,
                                         capFloorLengths_[i],
                                         index_,
                                         0.04, // dummy strike
                                         0*Days,
                                         dummy);
            boost::shared_ptr<FloatingRateCoupon> lFRC =
                                                temp.lastFloatingRateCoupon();
            optionletDates_[i] = lFRC->fixingDate();
            optionletPaymentDates_[i] = lFRC->date();
            optionletAccrualPeriods_[i] = lFRC->accrualPeriod();
            optionletTimes_[i] =
                dc.yearFraction(referenceDate, optionletDates_[i]);
            atmOptionletRate_[i] = index_->forecastFixing(optionletDates_[i]);
            capFloors_[i].resize(nStrikes_);
        }

        // the switch strike might be the average of atmOptionletRate_
        
        for (Size j=0; j<nStrikes_; ++j) {
            // using out-of-the-money options
            CapFloor::Type capFloorType = strikes[j] < switchStrike_ ?
                                   CapFloor::Floor : CapFloor::Cap;
            Option::Type optionletType = capFloorType==CapFloor::Floor ?
                                   Option::Put : Option::Call;
            Real previousCapFloorPrice = 0.0;
            for (Size i=0; i<nOptionletTenors_; ++i) {
                capFloorVols_[i][j] = surface_->volatility(capFloorLengths_[i],
                                                           strikes[j],
                                                           true);
                boost::shared_ptr<BlackCapFloorEngine> engine(new
                                BlackCapFloorEngine(capFloorVols_[i][j], dc));
                capFloors_[i][j] = MakeCapFloor(capFloorType,
                                                capFloorLengths_[i], index_,
                                                strikes[j], 0*Days, engine);
                capFloorPrices_[i][j] = capFloors_[i][j]->NPV();
                optionletPrices_[i][j] = capFloorPrices_[i][j] -
                                                        previousCapFloorPrice;
                previousCapFloorPrice = capFloorPrices_[i][j];
                DiscountFactor d = capFloors_[i][j]->discountCurve()->discount(
                                                        optionletPaymentDates_[i]);
                DiscountFactor optionletAnnuity=optionletAccrualPeriods_[i]*d;
                try {
                    optionletStDevs_[i][j] =
                        blackFormulaImpliedStdDev(optionletType,
                                                  strikes[j],
                                                  atmOptionletRate_[i],
                                                  optionletPrices_[i][j],
                                                  optionletAnnuity,
                                                  optionletStDevs_[i][j]);
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
                optionletVols_[i][j] = optionletStDevs_[i][j] /
                                                std::sqrt(optionletTimes_[i]);
            }
        }

    }
    
    const std::vector<Rate>& OptionletStripper::strikes() const {
        return surface_->strikes();
    }

    boost::shared_ptr<CapFloorTermVolSurface> OptionletStripper::surface() const {
        return surface_;
    }
}
