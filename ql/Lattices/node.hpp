
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file node.hpp
    \brief Node class

    \fullpath
    ql/Lattices/%node.hpp
*/

// $Id$

#ifndef quantlib_lattices_node_h
#define quantlib_lattices_node_h

#include <ql/qldefines.hpp>
#include <ql/termstructure.hpp>

#include <iostream>
#include <list>
#include <vector>

namespace QuantLib {

    namespace Lattices {

        class Node {
          public:
            Node(unsigned int nDescendants, unsigned int i, int j) 
            : nDescendants_(nDescendants), descendants_(nDescendants), 
              ascendants_(0), ascendantBranches_(0), 
              probabilities_(nDescendants), i_(i), j_(j), 
              discount_(0.0), statePrice_(0.0), value_(0.0) {}

            double value() const { return value_; }
            void setValue(double value) { 
                value_ = value; 
            }

            unsigned int i() const { return i_; }
            int j() const { return j_; }

            double probability(unsigned int branch) const {
                return probabilities_[branch];
            }
            void setProbability(double prob, unsigned int branch) {
                probabilities_[branch] = prob;
            }
            Node& ascendant(unsigned int index) {
                return *(ascendants_[index]);
            }
            Node& descendant(unsigned int branch) { 
                return *(descendants_[branch]);
            }
            unsigned int ascendantBranch(unsigned int index) const {
                return ascendantBranches_[index];
            }
            unsigned int nAscendants() const {
                return ascendants_.size();
            }
            void setDescendant(Node& node, unsigned int branch) {
                descendants_[branch] = &node; 
                node.addAscendant(this, branch);
            }

            double discount() const { return discount_; }
            void setDiscount(double discount) { discount_ = discount; }
 
            void setStatePrice(double price) { statePrice_ = price; }
            double statePrice() const { return statePrice_; }

          private:
            void addAscendant(Node* node, unsigned int branch) {
                ascendants_.push_back(node);
                ascendantBranches_.push_back(branch);
            }

            unsigned int nDescendants_;
            std::vector<Node*> descendants_;
            std::vector<Node*> ascendants_;
            std::vector<unsigned int> ascendantBranches_;
            std::vector<double> probabilities_;

            unsigned int i_;
            int j_;

            double discount_;
            double statePrice_;
            double value_;

        };

    }

}

#endif
