
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/Swaption/treeswaptionengine.hpp>
#include <ql/PricingEngines/Swaption/discretizedswaption.hpp>

namespace QuantLib {

    TreeSwaptionEngine::TreeSwaptionEngine(
                               const boost::shared_ptr<ShortRateModel>& model,
                               Size timeSteps) 
    : LatticeShortRateModelEngine<Swaption::arguments, Swaption::results> 
    (model, timeSteps) {} 

    TreeSwaptionEngine::TreeSwaptionEngine(
                               const boost::shared_ptr<ShortRateModel>& model,
                               const TimeGrid& timeGrid) 
    : LatticeShortRateModelEngine<Swaption::arguments, Swaption::results> 
    (model, timeGrid) {}

    void TreeSwaptionEngine::calculate() const {

        QL_REQUIRE(model_, "no model specified");
        boost::shared_ptr<Lattice> lattice;

        if (!lattice_) {
            std::list<Time> times;
            Time t;
            Size i;
            for (i=0; i<arguments_.stoppingTimes.size(); i++) {
                t = arguments_.stoppingTimes[i];
                if (t >= 0.0)
                    times.push_back(t);
            }
            for (i=0; i<arguments_.fixedResetTimes.size(); i++) {
                t = arguments_.fixedResetTimes[i];
                if (t >= 0.0)
                    times.push_back(t);
            }
            for (i=0; i<arguments_.fixedPayTimes.size(); i++) {
                t = arguments_.fixedPayTimes[i];
                if (t >= 0.0)
                    times.push_back(t);
            }
            for (i=0; i<arguments_.floatingResetTimes.size(); i++) {
                t = arguments_.floatingResetTimes[i];
                if (t >= 0.0)
                    times.push_back(t);
            }
            for (i=0; i<arguments_.floatingPayTimes.size(); i++) {
                t = arguments_.floatingPayTimes[i];
                if (t >= 0.0)
                    times.push_back(t);
            }

            TimeGrid timeGrid(times.begin(), times.end(), timeSteps_);
            lattice = model_->tree(timeGrid);
        } else {
            lattice = lattice_;
        }


        boost::shared_ptr<DiscretizedSwap> swap(
                                    new DiscretizedSwap(lattice, arguments_));
        Time lastFixedPay = arguments_.fixedPayTimes.back();
        Time lastFloatPay = arguments_.floatingPayTimes.back();
        swap->initialize(lattice, QL_MAX(lastFixedPay,lastFloatPay));

        DiscretizedSwaption swaption(swap, arguments_);
        swaption.initialize(lattice, arguments_.stoppingTimes.back());

        Time nextExercise = 
            *std::find_if(arguments_.stoppingTimes.begin(), 
                          arguments_.stoppingTimes.end(),
                          std::bind2nd(std::greater_equal<Time>(), 0.0));
        swaption.rollback(nextExercise);

        results_.value = swaption.presentValue();
    }

}
