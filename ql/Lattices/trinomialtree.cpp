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

#include <cmath>

namespace QuantLib {

    namespace Lattices {

        TrinomialTree::TrinomialTree(const Handle<DiffusionProcess>& process,
                                     const TimeGrid& timeGrid,
                                     bool isPositive)
        : Lattices::Tree(3), process_(process) {

            nodes_.push_back(Column());
            nodes_[0].push_back(new TrinomialNode(0));
            nodes_[0][0]->statePrice() = 1.0;
            
            t_ = timeGrid;

            double x0 = process->x0();

            Size nTimeSteps = t_.size() - 1;
            for (Size i=0; i<nTimeSteps; i++) {

                //Determine branching
                double v = QL_SQRT(process->variance(t(i), 0.0, dt(i)));
                dx_.push_back(v*QL_SQRT(3));

                std::vector<int> k(0);
                int j;
                for (j=jMin(i); j<=jMax(i); j++) {
                    double x = x0 + j*dx(i);
                    double m = process->expectation(t(i), x, dt(i));
                    int temp = (int)std::floor((m-x0)/dx(i+1) + 0.5);
                    if (isPositive) {
                        while (x0+(temp-1)*dx(i+1)<=0) {
                            temp++;
                            std::cout << "pushing up to " << temp << std::endl;
                        }
                    }
                    k.push_back(temp);
                    double e = m - (x0+temp*dx(i+1));
                    trinode(i,j)->setValues(e, v);
                }
                k_.push_back(k);
                if (std::adjacent_find(k.begin(),k.end(), std::greater<int>()) 
                    != k.end())
                    std::cout << "Warning: link vector unsorted" << std::endl;
//              QL_REQUIRE(std::adjacent_find(k.begin(),k.end(),
//                         std::greater<int>()) == k.end(),
//                         "Link vector unsorted!");
                nodes_.push_back(Column());

                int jMin = *std::min_element(k.begin(), k.end()) - 1;
                int jMax = *std::max_element(k.begin(), k.end()) + 1;

                for (j=jMin; j<=jMax; j++) {
                    nodes_.back().push_back(new TrinomialNode(j));
                }

            }
        }

    }

}

