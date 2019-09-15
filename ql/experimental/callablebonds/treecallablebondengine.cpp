/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo

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

#include <ql/models/shortrate/onefactormodel.hpp>
#include <ql/experimental/callablebonds/treecallablebondengine.hpp>
#include <ql/experimental/callablebonds/discretizedcallablefixedratebond.hpp>

namespace QuantLib {

    TreeCallableFixedRateBondEngine::TreeCallableFixedRateBondEngine(
                               const ext::shared_ptr<ShortRateModel>& model,
                               const Size timeSteps,
                               const Handle<YieldTermStructure>& termStructure)
    : LatticeShortRateModelEngine<CallableBond::arguments,
                                  CallableBond::results>(model, timeSteps),
      termStructure_(termStructure) {
        registerWith(termStructure_);
    }

    TreeCallableFixedRateBondEngine::TreeCallableFixedRateBondEngine(
                               const ext::shared_ptr<ShortRateModel>& model,
                               const TimeGrid& timeGrid,
                               const Handle<YieldTermStructure>& termStructure)
    : LatticeShortRateModelEngine<CallableBond::arguments,
                                  CallableBond::results>(model, timeGrid),
      termStructure_(termStructure) {
        registerWith(termStructure_);
    }

    void TreeCallableFixedRateBondEngine::calculate() const {
        return calculateWithSpread(arguments_.spread);
    }

    void TreeCallableFixedRateBondEngine::calculateWithSpread(Spread s) const {
        QL_REQUIRE(!model_.empty(), "no model specified");

        Date referenceDate;
        DayCounter dayCounter;

        ext::shared_ptr<TermStructureConsistentModel> tsmodel =
            ext::dynamic_pointer_cast<TermStructureConsistentModel>(*model_);
        if (tsmodel) {
            referenceDate = tsmodel->termStructure()->referenceDate();
            dayCounter = tsmodel->termStructure()->dayCounter();
        } else {
            referenceDate = termStructure_->referenceDate();
            dayCounter = termStructure_->dayCounter();
        }

        DiscretizedCallableFixedRateBond callableBond(arguments_,
                                                      referenceDate,
                                                      dayCounter);
        ext::shared_ptr<Lattice> lattice;

        if (lattice_) {
            lattice = lattice_;
        } else {
            std::vector<Time> times = callableBond.mandatoryTimes();
            TimeGrid timeGrid(times.begin(), times.end(), timeSteps_);
            lattice = model_->tree(timeGrid);
        }

        if (s != 0.0) {
            OneFactorModel::ShortRateTree *sr=
                dynamic_cast<OneFactorModel::ShortRateTree*>(&(*lattice));
            QL_REQUIRE(sr,
                       "Spread is not supported for trees other than OneFactorModel");
            sr->setSpread(s);
        }

        Time redemptionTime =
            dayCounter.yearFraction(referenceDate,
                                    arguments_.redemptionDate);
        callableBond.initialize(lattice, redemptionTime);
        callableBond.rollback(0.0);
        results_.value = results_.settlementValue = callableBond.presentValue();
    }

}

