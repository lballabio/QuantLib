
/*
 Copyright (C) 2000, 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file node.hpp
    \brief Node class

    \fullpath
    ql/Lattices/%node.hpp
*/

// $Id$

#ifndef quantlib_lattices_node_h
#define quantlib_lattices_node_h

#include <ql/termstructure.hpp>

#include <iostream>

namespace QuantLib {

    namespace Lattices {

        class Node {
          public:
            Node(Size nDescendants, Size i, int j)
            : descendants_(nDescendants), probabilities_(nDescendants),
              i_(i), j_(j), discount_(0.0), statePrice_(0.0) {}

            Size i() const { return i_; }
            int j() const { return j_; }

            double probability(Size branch) const {
                return probabilities_[branch];
            }
            void setProbability(double prob, Size branch) {
                probabilities_[branch] = prob;
            }
            Node& descendant(Size branch) {
                return *(descendants_[branch]);
            }
            const Node& descendant(Size branch) const {
                return *(descendants_[branch]);
            }
            void setDescendant(Node& node, Size branch) {
                descendants_[branch] = &node;
            }

            double discount() const { return discount_; }
            void setDiscount(double discount) { discount_ = discount; }

            double& statePrice() { return statePrice_; }
            const double& statePrice() const { return statePrice_; }

          private:
            std::vector<Node*> descendants_;
            std::vector<double> probabilities_;

            Size i_;
            int j_;

            double discount_;
            double statePrice_;
        };

    }

}

#endif
