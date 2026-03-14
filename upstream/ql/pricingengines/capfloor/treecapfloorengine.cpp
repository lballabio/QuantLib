/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/shortrate/onefactormodel.hpp>
#include <ql/pricingengines/capfloor/discretizedcapfloor.hpp>
#include <ql/pricingengines/capfloor/treecapfloorengine.hpp>
#include <utility>

namespace QuantLib {

    TreeCapFloorEngine::TreeCapFloorEngine(const ext::shared_ptr<ShortRateModel>& model,
                                           Size timeSteps,
                                           Handle<YieldTermStructure> termStructure)
    : LatticeShortRateModelEngine<CapFloor::arguments, CapFloor::results>(model, timeSteps),
      termStructure_(std::move(termStructure)) {
        registerWith(termStructure_);
    }

    TreeCapFloorEngine::TreeCapFloorEngine(const ext::shared_ptr<ShortRateModel>& model,
                                           const TimeGrid& timeGrid,
                                           Handle<YieldTermStructure> termStructure)
    : LatticeShortRateModelEngine<CapFloor::arguments, CapFloor::results>(model, timeGrid),
      termStructure_(std::move(termStructure)) {
        registerWith(termStructure_);
    }

    void TreeCapFloorEngine::calculate() const {

        QL_REQUIRE(!model_.empty(), "no model specified");

        Date referenceDate;
        DayCounter dayCounter;

        ext::shared_ptr<TermStructureConsistentModel> tsmodel =
            ext::dynamic_pointer_cast<TermStructureConsistentModel>(*model_);
        if (tsmodel != nullptr) {
            referenceDate = tsmodel->termStructure()->referenceDate();
            dayCounter = tsmodel->termStructure()->dayCounter();
        } else {
            referenceDate = termStructure_->referenceDate();
            dayCounter = termStructure_->dayCounter();
        }

        DiscretizedCapFloor capfloor(arguments_, referenceDate, dayCounter);
        ext::shared_ptr<Lattice> lattice;

        if (lattice_ != nullptr) {
            lattice = lattice_;
        } else {
            std::vector<Time> times = capfloor.mandatoryTimes();
            TimeGrid timeGrid(times.begin(), times.end(), timeSteps_);
            lattice = model_->tree(timeGrid);
        }

        Time firstTime = dayCounter.yearFraction(referenceDate,
                                                 arguments_.startDates.front());
        Time lastTime = dayCounter.yearFraction(referenceDate,
                                                arguments_.endDates.back());
        capfloor.initialize(lattice, lastTime);
        capfloor.rollback(firstTime);

        results_.value = capfloor.presentValue();
    }

}


