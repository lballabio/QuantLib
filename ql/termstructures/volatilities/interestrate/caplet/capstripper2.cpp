/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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

namespace QuantLib {

    CapletStripper::CapletStripper(
                    const boost::shared_ptr<CapVolatilitySurface>& surface,
                    const boost::shared_ptr<IborIndex>& index)
    : surface_(surface), index_(index), nStrikes_(surface->strikes().size())
    {
        registerWith(surface);
        registerWith(index);
        registerWith(Settings::instance().evaluationDate());

        Period indexTenor = index->tenor();
        Period maxSurfaceTenor = surface->optionTenors().back();
        optionTenors_.push_back(indexTenor);
        while (optionTenors_.back()<maxSurfaceTenor)
            optionTenors_.push_back(optionTenors_.back()+indexTenor);

        capPrices_ = Matrix(nOptionTenors_, nStrikes_);
        capletPrices_ = Matrix(nOptionTenors_, nStrikes_);
        capVols_ = Matrix(nOptionTenors_, nStrikes_);
        capletVols_ = Matrix(nOptionTenors_, nStrikes_);
        capletStDevs_ = Matrix(nOptionTenors_, nStrikes_);
        atmCapletRate = std::vector<Rate>(nOptionTenors_);
        optionDates_ = std::vector<Date>(nOptionTenors_);
        optionTimes_ = std::vector<Time>(nOptionTenors_);
        caps_ = CapMatrix(nOptionTenors_);

    }

    void CapletStripper::performCalculations() const {

        Date evaluationDate = Settings::instance().evaluationDate();
        const std::vector<Period>& tenors = surface_->optionTenors();
        const std::vector<Rate>& strikes = surface_->strikes();

        const Calendar& cal = index_->fixingCalendar();
        const DayCounter& dc = surface_->dayCounter();
        for (Size i=0; i<nOptionTenors_; ++i) {
            boost::shared_ptr<BlackCapFloorEngine> dummy(new
                                        BlackCapFloorEngine(0.20, dc));
            CapFloor temp = MakeCapFloor(CapFloor::Cap, tenors[i], index_,
                                         0.04, // dummy strike
                                         0*Days,
                                         dummy // dummy vol
                                         );
            //atmCapRate[i] = temp.atmRate();
            optionDates_[i] = temp.lastFixingDate();
            optionTimes_[i] = dc.yearFraction(evaluationDate, optionDates_[i]);

            atmCapletRate[i] = index_->fixing(optionDates_[i]);
            caps_[i].resize(nStrikes_);
        }

        Spread strikeRange = strikes.back()-strikes.front();
        Rate switchStrike = strikes.front()+0.5*strikeRange;

        for (Size j=0; j<nStrikes_; ++j) {
            Real runningNPV = 0.0;
            CapFloor::Type type = strikes[j] < switchStrike ?
                                   CapFloor::Floor : CapFloor::Cap;
            Option::Type optionType = type==CapFloor::Floor ?
                                   Option::Put : Option::Call;
            for (Size i=0; i<nOptionTenors_; ++i) {
                capVols_[i][j] = surface_->volatility(optionTimes_[i], strikes[j]);
                boost::shared_ptr<BlackCapFloorEngine> engine(new
                                BlackCapFloorEngine(capVols_[i][j], dc));
                caps_[i][j] = MakeCapFloor(type, tenors[i], index_,
                                           strikes[j], 0*Days, engine);
                capPrices_[i][j] = caps_[i][j]->NPV();
                capletPrices_[i][j] = capPrices_[i][j]-runningNPV;
                runningNPV = capPrices_[i][j];
                DiscountFactor capletAnnuity = 0.5*1.0; // FIXME
                capletStDevs_[i][j] = blackFormulaImpliedStdDev(optionType,
                                                                strikes[j],
                                                                atmCapletRate[i],
                                                                capletPrices_[i][j],
                                                                capletAnnuity,
                                                                capVols_[i][j]);
                capletVols_[i][j] = capletStDevs_[i][j] /
                                                std::sqrt(optionTimes_[i]);

            }
        }

    }
    
    const std::vector<Rate>& CapletStripper::strikes() const {
        return surface_->strikes();
    }
    
}
