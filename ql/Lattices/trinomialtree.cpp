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
                                     const TimeGrid& timeGrid)
        : Lattices::Tree(3) {

            t_ = timeGrid;

            //adjust space intervals
            dx_.resize(t_.size());
            dx_[0] = 0.0; //Just one node
            Size i;
            for (i=0; i<(dx_.size()-1); i++) {
                //The diffusion term must be r-independant
                double v = QL_SQRT(process->variance(t(i), 0.0, dt(i)));
                dx_[i+1] = v*QL_SQRT(3);
            }

            Size nTimeSteps = t_.size() - 1;
            for (i=0; i<nTimeSteps; i++) {

                //Determine branching
                double v = dx(i+1)/QL_SQRT(3);
                double v2 = v*v;

                std::vector<int> k(0);
                for (int j=jMin(i); j<=jMax(i); j++) {
                    double x = j*dx(i);
                    double m = process->expectation(t(i), x, dt(i));
                    k.push_back(findCentralNode(i,j,m));
                    double e = m - k.back()*dx(i+1);
                    double e2 = e*e;
                    double e3 = e*QL_SQRT(3);

                    node(i,j).probability[0] = (1.0 + e2/v2 - e3/v)/6.0;
                    node(i,j).probability[1] = (2.0 - e2/v2)/3.0;
                    node(i,j).probability[2] = (1.0 + e2/v2 + e3/v)/6.0;
                }

                addLevel(k);
            }
        }

        int TrinomialTree::findCentralNode(Size i, int j, double avg) const {
            return (int)floor(avg/dx(i+1) + 0.5);
        }

        void TrinomialTree::addLevel(const std::vector<int>& k) {
            
            QL_REQUIRE(std::adjacent_find(k.begin(),k.end(),
                           std::greater<int>()) == k.end(),
                       "Link vector unsorted!");
            Size i = nodes_.size();
            nodes_.push_back(std::vector<Node>());

            int jMin = k.front() - 1;
            int jMax = k.back() + 1;

            for (int j=jMin; j<=jMax; j++) {
                nodes_[i].push_back(Node(3, j));
            }

            QL_REQUIRE(k.size()==nodes_[i-1].size(), "Error!!!");
            for (Size l=0; l<k.size(); l++) {
                nodes_[i-1][l].descendant[0] = k[l] - 1;
                nodes_[i-1][l].descendant[1] = k[l];
                nodes_[i-1][l].descendant[2] = k[l] + 1;
            }
        }

    }

}

