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
/*! \file trinomialtree.cpp
    \brief Trinomial tree class

    \fullpath
    ql/Lattices/%trinomialtree.cpp
*/

// $Id$

#include "ql/Lattices/trinomialtree.hpp"

namespace QuantLib {

    namespace Lattices {

        TrinomialTree::TrinomialTree(const Handle<DiffusionProcess>& process,
                                     const TimeGrid& timeGrid,
                                     bool isPositive)
        : Lattices::Tree(timeGrid, 3), dx_(1, 0.0) {

            columns_.push_back(Column(1));
            columns_[0].addToStatePrice(0, 1.0);
            
            t_ = timeGrid;

            double x0 = process->x0();

            Size nTimeSteps = t_.size() - 1;
            int jMin = 0;
            int jMax = 0;

            for (Size i=0; i<nTimeSteps; i++) {
                Time t = t_[i];
                Time dt = t_.dt(i);

                //Variance must be independent of x
                double v2 = process->variance(t, 0.0, dt);
                double v = QL_SQRT(v2);
                dx_.push_back(v*QL_SQRT(3));

                Handle<TrinomialBranching> branching(new TrinomialBranching());
                for (int j=jMin; j<=jMax; j++) {
                    double x = x0 + j*dx_[i];
                    double m = process->expectation(t, x, dt);
                    int temp = (int)QL_FLOOR((m-x0)/dx_[i+1] + 0.5);

                    if (isPositive) {
                        while (x0+(temp-1)*dx_[i+1]<=0) {
                            temp++;
                        }
                    }

                    branching->k_.push_back(temp);
                    double e = m - (x0 + temp*dx_[i+1]);
                    double e2 = e*e;
                    double e3 = e*QL_SQRT(3);

                    branching->probs_[0].push_back((1.0 + e2/v2 - e3/v)/6.0);
                    branching->probs_[1].push_back((2.0 - e2/v2)/3.0);
                    branching->probs_[2].push_back((1.0 + e2/v2 + e3/v)/6.0);
                }
                columns_[i].setBranching(branching);

                const std::vector<int>& k = branching->k_;
                jMin = *std::min_element(k.begin(), k.end()) - 1;
                jMax = *std::max_element(k.begin(), k.end()) + 1;
                Size width = jMax - jMin + 1;

                columns_.push_back(Column(width));
            }

        }

    }

}

