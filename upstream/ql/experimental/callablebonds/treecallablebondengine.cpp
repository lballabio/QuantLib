/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo
 Copyright (C) 2021 Ralf Konrad Eckel

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

#include <ql/experimental/callablebonds/discretizedcallablefixedratebond.hpp>
#include <ql/experimental/callablebonds/treecallablebondengine.hpp>
#include <ql/models/shortrate/onefactormodel.hpp>
#include <utility>

namespace QuantLib {

    TreeCallableFixedRateBondEngine::TreeCallableFixedRateBondEngine(
        const ext::shared_ptr<ShortRateModel>& model,
        const Size timeSteps,
        Handle<YieldTermStructure> termStructure)
    : LatticeShortRateModelEngine<CallableBond::arguments, CallableBond::results>(model, timeSteps),
      termStructure_(std::move(termStructure)) {
        registerWith(termStructure_);
    }

    TreeCallableFixedRateBondEngine::TreeCallableFixedRateBondEngine(
        const ext::shared_ptr<ShortRateModel>& model,
        const TimeGrid& timeGrid,
        Handle<YieldTermStructure> termStructure)
    : LatticeShortRateModelEngine<CallableBond::arguments, CallableBond::results>(model, timeGrid),
      termStructure_(std::move(termStructure)) {
        registerWith(termStructure_);
    }

    void TreeCallableFixedRateBondEngine::calculate() const {
        calculateWithSpread(arguments_.spread);
    }

    void TreeCallableFixedRateBondEngine::calculateWithSpread(Spread s) const {
        QL_REQUIRE(!model_.empty(), "no model specified");

        ext::shared_ptr<TermStructureConsistentModel> tsmodel =
            ext::dynamic_pointer_cast<TermStructureConsistentModel>(*model_);
        Handle<YieldTermStructure> discountCurve =
            tsmodel != nullptr ? tsmodel->termStructure() : termStructure_;

        DiscretizedCallableFixedRateBond callableBond(arguments_, discountCurve);
        ext::shared_ptr<Lattice> lattice;

        if (lattice_ != nullptr) {
            lattice = lattice_;
        } else {
            std::vector<Time> times = callableBond.mandatoryTimes();
            TimeGrid timeGrid(times.begin(), times.end(), timeSteps_);
            lattice = model_->tree(timeGrid);
        }

        if (s != 0.0) {
            auto* sr = dynamic_cast<OneFactorModel::ShortRateTree*>(&(*lattice));
            QL_REQUIRE(sr,
                       "Spread is not supported for trees other than OneFactorModel");
            sr->setSpread(s);
        }

        auto referenceDate = discountCurve->referenceDate();
        auto dayCounter = discountCurve->dayCounter();
        Time redemptionTime = dayCounter.yearFraction(referenceDate, arguments_.redemptionDate);

        callableBond.initialize(lattice, redemptionTime);
        callableBond.rollback(0.0);

        results_.value = callableBond.presentValue();

        DiscountFactor d = discountCurve->discount(arguments_.settlementDate);
        results_.settlementValue = results_.value / d;
    }

}

