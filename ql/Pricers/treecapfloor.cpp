
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

/*! \file treecapfloor.cpp
    \brief Cap/Floor calculated using a tree
*/

#include "ql/Pricers/treecapfloor.hpp"
#include "ql/ShortRateModels/onefactormodel.hpp"
#include <ql/Pricers/capfloorpricer.hpp>

namespace QuantLib {

    namespace Pricers {

        using namespace Instruments;
        using namespace ShortRateModels;
        using namespace Lattices;
        using namespace PricingEngines;

        TreeCapFloor::TreeCapFloor(const Handle<Model>& model, Size timeSteps) 
        : LatticeShortRateModelEngine<CapFloor::arguments, 
                                      CapFloor::results >(model,timeSteps){}

        TreeCapFloor::TreeCapFloor(const Handle<Model>& model,
                                   const TimeGrid& timeGrid) 
        : LatticeShortRateModelEngine<CapFloor::arguments, 
                                      CapFloor::results>(model,timeGrid) {}

        void TreeCapFloor::calculate() const {

            QL_REQUIRE(!model_.isNull(), "TreeCapFloor: No model specified");
            Handle<Lattice> lattice;

            if (lattice_.isNull()) {
                std::list<Time> times(0);
                Size nPeriods = arguments_.startTimes.size();
                Size i;
                for (i=0; i<nPeriods; i++) {
                    times.push_back(arguments_.startTimes[i]);
                    times.push_back(arguments_.endTimes[i]);
                }

                TimeGrid timeGrid(times.begin(), times.end(), timeSteps_);
                lattice = model_->tree(timeGrid);
            } else {
                lattice = lattice_;
            }

            Handle<DiscretizedAsset> capfloor(
                new DiscretizedCapFloor(lattice,arguments_));

            lattice->initialize(capfloor, arguments_.endTimes.back());
            lattice->rollback(capfloor, arguments_.startTimes.front());

            results_.value = lattice->presentValue(capfloor);
        }

    }

}
