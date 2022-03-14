/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005, 2007 StatPro Italia srl

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

#include <ql/math/functional.hpp>
#include <ql/pricingengines/swaption/discretizedswaption.hpp>
#include <ql/pricingengines/swaption/treeswaptionengine.hpp>
#include <utility>

namespace QuantLib {

    TreeSwaptionEngine::TreeSwaptionEngine(const ext::shared_ptr<ShortRateModel>& model,
                                           Size timeSteps,
                                           Handle<YieldTermStructure> termStructure)
    : LatticeShortRateModelEngine<Swaption::arguments, Swaption::results>(model, timeSteps),
      termStructure_(std::move(termStructure)) {
        registerWith(termStructure_);
    }

    TreeSwaptionEngine::TreeSwaptionEngine(const ext::shared_ptr<ShortRateModel>& model,
                                           const TimeGrid& timeGrid,
                                           Handle<YieldTermStructure> termStructure)
    : LatticeShortRateModelEngine<Swaption::arguments, Swaption::results>(model, timeGrid),
      termStructure_(std::move(termStructure)) {
        registerWith(termStructure_);
    }

    TreeSwaptionEngine::TreeSwaptionEngine(const Handle<ShortRateModel>& model,
                                           Size timeSteps,
                                           Handle<YieldTermStructure> termStructure)
    : LatticeShortRateModelEngine<Swaption::arguments, Swaption::results>(model, timeSteps),
      termStructure_(std::move(termStructure)) {
        registerWith(termStructure_);
    }

    void TreeSwaptionEngine::calculate() const {

        QL_REQUIRE(arguments_.settlementMethod != Settlement::ParYieldCurve,
                   "cash settled (ParYieldCurve) swaptions not priced with "
                   "TreeSwaptionEngine");
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

        DiscretizedSwaption swaption(arguments_, referenceDate, dayCounter);
        ext::shared_ptr<Lattice> lattice;

        if (lattice_ != nullptr) {
            lattice = lattice_;
        } else {
            std::vector<Time> times = swaption.mandatoryTimes();
            TimeGrid timeGrid(times.begin(), times.end(), timeSteps_);
            lattice = model_->tree(timeGrid);
        }

        std::vector<Time> stoppingTimes(arguments_.exercise->dates().size());
        for (Size i=0; i<stoppingTimes.size(); ++i)
            stoppingTimes[i] =
                dayCounter.yearFraction(referenceDate,
                                        arguments_.exercise->date(i));

        swaption.initialize(lattice, stoppingTimes.back());

        Time nextExercise =
            *std::find_if(stoppingTimes.begin(),
                          stoppingTimes.end(),
                          greater_or_equal_to<Time>(0.0));
        swaption.rollback(nextExercise);

        results_.value = swaption.presentValue();
    }

}
