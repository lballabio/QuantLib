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

/*
        BinomialTree::BinomialTree(const TimeGrid& timeGrid)
        : Lattices::Tree(2) {

            t_ = timeGrid;

            //adjust space intervals
            dx_.resize(t_.size());
            dx_[0] = 0.0; //Just one node
            Size i;
            double dx = QL_SQRT(dt(0));
            for (i=0; i<(dx_.size()-1); i++) {
                dx_[i+1] = dx;
            }

            Size nTimeSteps = t_.size() - 1;
            for (i=0; i<nTimeSteps; i++) {

                //Determine branching
                double v = dx(i+1)/QL_SQRT(3);
                double v2 = v*v;

                for (int j=jMin(i); j<=jMax(i); j++) {
                    double x = j*dx(i);
                    double m = process->expectation(t(i), x, dt(i));
                    double e = m - k.back()*dx(i+1);
                    double e2 = e*e;
                    double e3 = e*QL_SQRT(3);

                    node(i,j).probability[0] = (1.0 + e2/v2 - e3/v)/6.0;
                    node(i,j).probability[1] = (2.0 - e2/v2)/3.0;
                }

                addLevel(k);
            }
        }
*/
        void BinomialTree::addLevel() {
            
            int i = nodes_.size();
            nodes_.push_back(std::vector<Node>());

            int j;
            for (j=-i; j<=i; j+=2) {
                nodes_[i].push_back(Node(2, j));
            }
            for (j=-(i-1); j<=(i-1); j+=2) {
                node(i-1,j).descendant[0] = j-1;
                node(i-1,j).descendant[1] = j+1;
            }
        }

    }

}

