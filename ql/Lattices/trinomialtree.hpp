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
/*! \file trinomialtree.hpp
    \brief Trinomial tree class

    \fullpath
    ql/Lattices/%trinomialtree.hpp
*/

// $Id$

#ifndef quantlib_lattices_trinomial_tree_h
#define quantlib_lattices_trinomial_tree_h

#include <ql/diffusionprocess.hpp>
#include <ql/Lattices/tree.hpp>

namespace QuantLib {

    namespace Lattices {

        class TrinomialNode : public Node {
          public:
            TrinomialNode(int j) : Node(j) {}

            virtual double probability(Size branch) const {
                switch (branch) {
                  case 0: return  (1.0 + e2v2_ - e3v_)/6.0;
                  case 1: return  (2.0 - e2v2_)/3.0;
                  case 2: return  (1.0 + e2v2_ + e3v_)/6.0;
                  default: throw Error("Invalid branch number");
                }
            }
            void setValues(double e, double v) {
                e2v2_ = (e*e)/(v*v);
                e3v_ = (e*QL_SQRT(3))/v;
            }
          private:
            double e2v2_, e3v_;
        };

        class TrinomialTree : public Tree {
          public:
            TrinomialTree() : Tree(3) {}

            TrinomialTree(const Handle<DiffusionProcess>& process,
                          const TimeGrid& timeGrid,
                          bool isPositive = false);
          protected:
            virtual Size nodeIndex(Size i, int j) const {
                return j - nodes_[i][0]->j();
            }
            virtual Node& descendant(Size i, int j, Size branch) {
                return node(i+1, k_[i][nodeIndex(i,j)]-1+branch);
            }
            virtual const Node& descendant(Size i, int j, Size branch) const {
                return node(i+1, k_[i][nodeIndex(i,j)]-1+branch);
            }
            TrinomialNode* trinode(Size i, int j) {
                return dynamic_cast<TrinomialNode*>(nodes_[i][nodeIndex(i,j)]);
            }

            //! Returns jMin
            int jMin(Size i) const { return nodes_[i].front()->j(); }

            //! Returns jMax
            int jMax(Size i) const { return nodes_[i].back()->j(); }

          private:
            Handle<DiffusionProcess> process_;
            std::vector<std::vector<int> > k_;
        };

    }

}

#endif
