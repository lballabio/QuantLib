/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file treeswaption.cpp
    \brief European swaption calculated using finite differences

    \fullpath
    ql/Pricers/%treeswaption.cpp
*/

// $Id$

#include "ql/Pricers/treeswaption.hpp"

namespace QuantLib {

    namespace Pricers {

        using namespace ShortRateModels;
        using namespace Lattices;

        TreeSwaption::TreeSwaption(
            const Handle<ShortRateModels::Model>& model,
            Size timeSteps) 
        : SwaptionPricer<ShortRateModels::Model>(model), 
          timeSteps_(timeSteps) {} 

        TreeSwaption::TreeSwaption(
            const Handle<ShortRateModels::Model>& model,
            const TimeGrid& timeGrid) 
        : SwaptionPricer<ShortRateModels::Model>(model), 
          timeGrid_(timeGrid), timeSteps_(0) {
            tree_ = model_->tree(timeGrid);
        }

        void TreeSwaption::update() {
            if (timeGrid_.size() > 0)
                tree_ = model_->tree(timeGrid_);
            notifyObservers();
        }

        void TreeSwaption::calculate() const {

            QL_REQUIRE(!model_.isNull(), "TreeSwaption: No model");
            Handle<Lattice> tree;

            if (tree_.isNull()) {
                std::list<Time> times(0);
                Size i;
                for (i=0; i<parameters_.exerciseTimes.size(); i++)
                    times.push_back(parameters_.exerciseTimes[i]);
                for (i=0; i<parameters_.fixedPayTimes.size(); i++)
                    times.push_back(parameters_.fixedPayTimes[i]);
                for (i=0; i<parameters_.floatingResetTimes.size(); i++)
                    times.push_back(parameters_.floatingResetTimes[i]);
                for (i=0; i<parameters_.floatingPayTimes.size(); i++)
                    times.push_back(parameters_.floatingPayTimes[i]);
                times.sort();
                times.unique();

                TimeGrid timeGrid(times, timeSteps_);
                tree = model_->tree(timeGrid);
            } else {
                tree = tree_;
            }

            Handle<DiscretizedAsset> swaption(
            new DiscretizedSwaption(tree, parameters_));

            tree->initialize(swaption, parameters_.exerciseTimes.back());
            tree->rollback(swaption, parameters_.exerciseTimes.front());

            results_.value = tree->presentValue(swaption);
        }

    }

}
