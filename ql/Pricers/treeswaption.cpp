
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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
    \brief European swaption computed on a lattice
*/

#include "ql/Pricers/treeswaption.hpp"
#include <ql/Pricers/swaptionpricer.hpp>

namespace QuantLib {

    namespace Pricers {

        using namespace ShortRateModels;
        using namespace Lattices;
        using namespace PricingEngines;
        using namespace Instruments;

        TreeSwaption::TreeSwaption(
            const Handle<ShortRateModels::Model>& model,
            Size timeSteps) 
        : LatticeShortRateModelEngine<SwaptionArguments, SwaptionResults> 
                (model, timeSteps) {} 

        TreeSwaption::TreeSwaption(
            const Handle<ShortRateModels::Model>& model,
            const TimeGrid& timeGrid) 
        : LatticeShortRateModelEngine<SwaptionArguments, SwaptionResults> 
                (model, timeGrid) {}

        void TreeSwaption::calculate() const {

            QL_REQUIRE(!model_.isNull(), "TreeSwaption: No model was specified");
            Handle<Lattice> lattice;

            if (lattice_.isNull()) {
                std::list<Time> times(0);
                Size i;
                for (i=0; i<arguments_.exerciseTimes.size(); i++)
                    times.push_back(arguments_.exerciseTimes[i]);

                for (i=0; i<arguments_.fixedResetTimes.size(); i++)
                    times.push_back(arguments_.fixedResetTimes[i]);

                for (i=0; i<arguments_.fixedPayTimes.size(); i++)
                    times.push_back(arguments_.fixedPayTimes[i]);

                for (i=0; i<arguments_.floatingResetTimes.size(); i++)
                    times.push_back(arguments_.floatingResetTimes[i]);

                for (i=0; i<arguments_.floatingPayTimes.size(); i++)
                    times.push_back(arguments_.floatingPayTimes[i]);

                TimeGrid timeGrid(times.begin(), times.end(), timeSteps_);
                lattice = model_->tree(timeGrid);
            } else {
                lattice = lattice_;
            }

            Handle<DiscretizedAsset> swaption(
                new DiscretizedSwaption(lattice,arguments_));

            lattice->initialize(swaption, arguments_.exerciseTimes.back());
            lattice->rollback(swaption, arguments_.exerciseTimes.front());

            results_.value = lattice->presentValue(swaption);
        }

    }

}
