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

#include <ql/pricingengines/swap/treeswapengine.hpp>
#include <ql/pricingengines/swap/discretizedswap.hpp>

namespace QuantLib {

    TreeVanillaSwapEngine::TreeVanillaSwapEngine(
                               const boost::shared_ptr<ShortRateModel>& model,
                              Size timeSteps,
                              const Handle<YieldTermStructure>& termStructure)
    : LatticeShortRateModelEngine<VanillaSwap::arguments,
                                  VanillaSwap::results>(model, timeSteps),
      termStructure_(termStructure) {
        registerWith(termStructure_);
    }

    TreeVanillaSwapEngine::TreeVanillaSwapEngine(
                               const boost::shared_ptr<ShortRateModel>& model,
                              const TimeGrid& timeGrid,
                              const Handle<YieldTermStructure>& termStructure)
    : LatticeShortRateModelEngine<VanillaSwap::arguments,
                                  VanillaSwap::results>(model, timeGrid),
      termStructure_(termStructure) {
        registerWith(termStructure_);
    }

    void TreeVanillaSwapEngine::calculate() const {

        QL_REQUIRE(!model_.empty(), "no model specified");

        Date referenceDate;
        DayCounter dayCounter;

        boost::shared_ptr<TermStructureConsistentModel> tsmodel =
            boost::dynamic_pointer_cast<TermStructureConsistentModel>(*model_);
        if (tsmodel) {
            referenceDate = tsmodel->termStructure()->referenceDate();
            dayCounter = tsmodel->termStructure()->dayCounter();
        } else {
            referenceDate = termStructure_->referenceDate();
            dayCounter = termStructure_->dayCounter();
        }

        DiscretizedSwap swap(arguments_, referenceDate, dayCounter);
        std::vector<Time> times = swap.mandatoryTimes();

        boost::shared_ptr<Lattice> lattice;
        if (lattice_) {
            lattice = lattice_;
        } else {
            TimeGrid timeGrid(times.begin(), times.end(), timeSteps_);
            lattice = model_->tree(timeGrid);
        }

        swap.initialize(lattice, times.back());
        swap.rollback(0.0);

        results_.value = swap.presentValue();
    }

}

