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
/*! \file treecapfloor.cpp
    \brief Cap/Floor calculated using a tree

    \fullpath
    ql/Pricers/%treecapfloor.cpp
*/

// $Id$

#include "ql/Pricers/treecapfloor.hpp"
#include "ql/InterestRateModelling/onefactormodel.hpp"

namespace QuantLib {

    namespace Pricers {

        using namespace Instruments;
        using namespace InterestRateModelling;
        using namespace Lattices;

        void TreeCapFloor::calculate() const {

            Handle<Tree> tree;

            if (tree_.isNull()) {
                QL_REQUIRE(!model_.isNull(), "Cannot price without model!");
                Handle<OneFactorModel> model(model_);

                std::list<Time> times(0);
                Size nPeriods = parameters_.startTimes.size();
                Size i;
                for (i=0; i<nPeriods; i++) {
                    times.push_back(parameters_.startTimes[i]);
                    times.push_back(parameters_.endTimes[i]);
                }
                times.sort();
                times.unique();

                TimeGrid timeGrid(times, timeSteps_);
                tree = model->tree(timeGrid);
            } else {
                tree = tree_;
            }

            Handle<NumericalDerivative> capfloor(
                new NumericalCapFloor(tree, parameters_));

            tree->initialize(capfloor, parameters_.endTimes.back());
            tree->rollback(capfloor, parameters_.startTimes.front());

            results_.value = tree->presentValue(capfloor);
        }

    }

}
