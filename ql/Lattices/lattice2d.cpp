
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

#include <ql/Lattices/lattice2d.hpp>

namespace QuantLib {

    Size Lattice2D::descendant(Size i, Size index, Size branch) const {
        Size modulo = tree1_->size(i);

        Size index1 = index % modulo;
        Size index2 = index / modulo;
        Size branch1 = branch % 3;
        Size branch2 = branch / 3;

        modulo = tree1_->size(i+1);
        return tree1_->descendant(i, index1, branch1) +
            tree2_->descendant(i, index2, branch2)*modulo;
    }

    double Lattice2D::probability(Size i, Size index, Size branch) const {
        Size modulo = tree1_->size(i);

        Size index1 = index % modulo;
        Size index2 = index / modulo;
        Size branch1 = branch % 3;
        Size branch2 = branch / 3;

        double prob1 = tree1_->probability(i, index1, branch1);
        double prob2 = tree2_->probability(i, index2, branch2);
        return prob1*prob2 + rho_*(m_[branch1][branch2])/36.0;
    }

    Lattice2D::Lattice2D(const boost::shared_ptr<TrinomialTree>& tree1,
                         const boost::shared_ptr<TrinomialTree>& tree2,
                         double correlation)
    : Lattice(tree1->timeGrid(), 9),
      tree1_(tree1), tree2_(tree2), m_(3,3), rho_(QL_FABS(correlation)) {

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

}

