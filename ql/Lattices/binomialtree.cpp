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
/*! \file binomialtree.cpp
    \brief Binomial tree class

    \fullpath
    ql/Lattices/%binomialtree.cpp
*/

// $Id$

#include "ql/Lattices/binomialtree.hpp"

namespace QuantLib {

    namespace Lattices {

        BinomialTree::BinomialTree(const Handle<DiffusionProcess>& process,
                                   Time end, Size steps)
        : Lattices::Tree(ConstantTimeGrid(end, steps), 2) {

            nodes_.push_back(Column(0, 1));
            nodes_[0].statePrice(0) = 1.0;

            double dt = end/steps;

            double x0 = process->x0();
            double dx = QL_SQRT(process->variance(t(i), 0.0, dt(i)));

            Size nTimeSteps = t_.size() - 1;
            for (Size i=0; i<nTimeSteps; i++) {
                //Determine branching
                Handle<BinomialBranching> branching(new BinomialBranching());
                column(i).setBranching(branching);
                nodes_.push_back(Column(i+1, i+2));
            }
        }

    }

}

