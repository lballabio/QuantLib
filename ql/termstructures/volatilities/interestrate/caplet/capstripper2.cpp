/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando ametrano
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

#include <ql/termstructures/volatilities/interestrate/caplet/capstripper2.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatilities/interestrate/cap/capvolsurface.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    OptionletStripper::OptionletStripper(
                    const boost::shared_ptr<CapVolatilitySurface>& surface,
                    const boost::shared_ptr<IborIndex>& index)
    : surface_(surface), index_(index), nStrikes_(surface->strikes().size())
    {
        registerWith(surface);
        registerWith(index);
        registerWith(Settings::instance().evaluationDate());

        Period indexTenor = index->tenor();
        Period maxCapFloorTenor = surface->optionTenors().back();

        // optionlet tenors and capfloor lengths
        optionletTenors_.push_back(indexTenor);
        capfloorLengths_.push_back(optionletTenors_.back()+indexTenor);
        QL_REQUIRE(maxCapFloorTenor>=capfloorLengths_.back(),
                   "too short capfloor term vol surface");
        while (capfloorLengths_.back()+indexTenor<=maxCapFloorTenor) {
            optionletTenors_.push_back(optionletTenors_.back()+indexTenor);
            capfloorLengths_.push_back(optionletTenors_.back()+indexTenor);
        }
        nOptionletTenors_ = optionletTenors_.size();

        capfloorPrices_ = Matrix(nOptionletTenors_, nStrikes_);
        optionletPrices_ = Matrix(nOptionletTenors_, nStrikes_);
        capfloorVols_ = Matrix(nOptionletTenors_, nStrikes_);
        optionletVols_ = Matrix(nOptionletTenors_, nStrikes_);
        optionletStDevs_ = Matrix(nOptionletTenors_, nStrikes_);
        atmOptionletRate = std::vector<Rate>(nOptionletTenors_);
        optionletDates_ = std::vector<Date>(nOptionletTenors_);
        optionletTimes_ = std::vector<Time>(nOptionletTenors_);
        capfloors_ = CapFloorMatrix(nOptionletTenors_);
    }

    void OptionletStripper::performCalculations() const {

        Date referenceDate = surface_->referenceDate();
        const std::vector<Rate>& strikes = surface_->strikes();
        const Calendar& cal = index_->fixingCalendar();
        const DayCounter& dc = surface_->dayCounter();
        for (Size i=0; i<nOptionletTenors_; ++i) {
            boost::shared_ptr<BlackCapFloorEngine> dummy(new
                                         BlackCapFloorEngine(0.20, dc));
            CapFloor temp = MakeCapFloor(CapFloor::Cap,
                                         capfloorLengths_[i],
                                         index_,
                                         0.04, // dummy strike
                                         0*Days,
                                         dummy);
            optionletDates_[i] = temp.lastFixingDate();
            optionletTimes_[i] = dc.yearFraction(referenceDate,
                                                 optionletDates_[i]);
            // force forecast ??
            atmOptionletRate[i] = index_->fixing(optionletDates_[i]);
            capfloors_[i].resize(nStrikes_);
        }

        Spread strikeRange = strikes.back()-strikes.front();
        Rate switchStrike = 0.5*strikeRange;
        //Rate switchStrike = strikes.front()+0.5*strikeRange;
        for (Size j=0; j<nStrikes_; ++j) {
            CapFloor::Type capFloorType = strikes[j] < switchStrike ?
                                   CapFloor::Floor : CapFloor::Cap;
            Option::Type optionletType = capFloorType==CapFloor::Floor ?
                                   Option::Put : Option::Call;
            Real previousCapFloorPrice = 0.0;
            for (Size i=0; i<nOptionletTenors_; ++i) {
                capfloorVols_[i][j] = surface_->volatility(capfloorLengths_[i],
                                                           strikes[j],
                                                           true);
                boost::shared_ptr<BlackCapFloorEngine> engine(new
                                BlackCapFloorEngine(capfloorVols_[i][j], dc));
                capfloors_[i][j] = MakeCapFloor(capFloorType,
                                                capfloorLengths_[i], index_,
                                                strikes[j], 0*Days, engine);
                capfloorPrices_[i][j] = capfloors_[i][j]->NPV();
                optionletPrices_[i][j] = capfloorPrices_[i][j] -
                                                        previousCapFloorPrice;
                previousCapFloorPrice = capfloorPrices_[i][j];
                DiscountFactor d = capfloors_[i][j]->discountCurve()->discount(
                                                        optionletDates_[i]);
                Real optionletAccrualPeriod = 0.5; //FIXME
                DiscountFactor optionletAnnuity = optionletAccrualPeriod*d;
                Real guess = capfloorVols_[i][j]*std::sqrt(optionletTimes_[i]);
                try {
                    optionletStDevs_[i][j] =
                        blackFormulaImpliedStdDev(optionletType,
                                                  strikes[j],
                                                  atmOptionletRate[i],
                                                  optionletPrices_[i][j],
                                                  optionletAnnuity,
                                                  guess);
                } catch (std::exception& e) {
                    QL_FAIL("could not bootstrap the optionlet:"
                            "\n date: " << optionletDates_[i] <<
                            "\n type: " << optionletType <<
                            "\n strike: " << io::rate(strikes[j]) <<
                            "\n atm: " << io::rate(atmOptionletRate[i]) <<
                            "\n price: " << optionletPrices_[i][j] <<
                            "\n annuity: " << optionletAnnuity <<
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
    
}
