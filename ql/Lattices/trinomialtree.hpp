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

        class TrinomialTree : public Tree {
          public:
            TrinomialTree(const Handle<DiffusionProcess>& process,
                          const TimeGrid& timeGrid,
                          bool isPositive = false);
            double dx(Size i) const { return dx_[i]; }
          protected:
            std::vector<double> dx_;
        };

        class TrinomialBranching : public Branching {
          public:
            TrinomialBranching() : probs_(3) {}
            virtual ~TrinomialBranching() {}

            Size descendant(Size index, Size branch) const;
            double probability(Size index, Size branch) const;
            int jMin() const;
          private:
            friend class TrinomialTree;

            std::vector<int> k_;
            std::vector<std::vector<double> > probs_;
        };

        inline 
        Size TrinomialBranching::descendant(Size index, Size branch) const {
            return (k_[index] - jMin()) - 1 + branch;
        }

        inline 
        double TrinomialBranching::probability(Size index, Size branch) const {
            return probs_[branch][index];
        }

        inline int TrinomialBranching::jMin() const {
            return *std::min_element(k_.begin(), k_.end()) - 1;
        }

    }

}

#endif
