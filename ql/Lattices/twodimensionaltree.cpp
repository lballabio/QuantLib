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
/*! \file twodimensionaltree.cpp
    \brief Trinomial tree class

    \fullpath
    ql/Lattices/%twodimensionaltree.cpp
*/

// $Id$

#include "ql/Lattices/twodimensionaltree.hpp"

namespace QuantLib {

    namespace Lattices {

        TwoDimensionalBranching::TwoDimensionalBranching(
            double correlation,
            const Handle<Branching>& branching1,
            const Handle<Branching>& branching2)
        : rho_(QL_FABS(correlation)), 
          branching1_(branching1), branching2_(branching2), m_(3, 3) {
            if (correlation < 0.0) {
                m_[0][0] = -1.0;
                m_[0][1] = -4.0;
                m_[0][2] =  5.0;
                m_[1][0] = -4.0;
                m_[1][1] =  8.0;
                m_[1][2] = -4.0;
                m_[2][0] =  5.0;
                m_[2][1] = -4.0;
                m_[2][2] = -1.0;
            } else {
                m_[0][0] =  5.0;
                m_[0][1] = -4.0;
                m_[0][2] = -1.0;
                m_[1][0] = -4.0;
                m_[1][1] =  8.0;
                m_[1][2] = -4.0;
                m_[2][0] = -1.0;
                m_[2][1] = -4.0;
                m_[2][2] =  5.0;
            }
        }

        Size TwoDimensionalBranching::descendant(Size index, 
                                                 Size branch) const {
            Size index1 = index % modulo_;
            Size index2 = index / modulo_;
            Size branch1 = branch % 3;
            Size branch2 = branch / 3;
            return branching1_->descendant(index1, branch1) +
                   branching2_->descendant(index2, branch2)*nextModulo_;
        }

        double TwoDimensionalBranching::probability(Size index, 
                                                    Size branch) const {
            Size index1 = index % modulo_;
            Size index2 = index / modulo_;
            Size branch1 = branch % 3;
            Size branch2 = branch / 3;
            double prob1 = branching1_->probability(index1, branch1);
            double prob2 = branching2_->probability(index2, branch2);
            return prob1*prob2 + rho_*(m_[branch1][branch2])/36.0;
        }

        TwoDimensionalTree::TwoDimensionalTree(
            const Handle<TrinomialTree>& tree1,
            const Handle<TrinomialTree>& tree2,
            double correlation)
        : Lattices::Tree(tree1->timeGrid(), 9) {

            columns_.push_back(Column(1));
            columns_[0].addToStatePrice(0, 1.0);
            
            Size nTimeSteps = t_.size() - 1;
            for (Size i=0; i<nTimeSteps; i++) {
                //Determine branching
                Handle<TwoDimensionalBranching> branching(
                    new TwoDimensionalBranching(correlation,
                                                tree1->column(i).branching(),
                                                tree2->column(i).branching()));
                branching->setModulo(tree1->column(i).size());
                branching->setNextModulo(tree1->column(i+1).size());
                columns_[i].setBranching(branching);

                Size width = tree1->column(i+1).size()*
                             tree2->column(i+1).size();
                columns_.push_back(Column(width));

            }
        }

    }

}

