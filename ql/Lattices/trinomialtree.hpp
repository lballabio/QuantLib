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
            TrinomialTree() : Tree(3) {}

            TrinomialTree(const Handle<DiffusionProcess>& process,
                          const TimeGrid& timeGrid);
          protected:
            virtual int findCentralNode(Size i, int j, double avg) const;

            void addLevel(const std::vector<int>& k);

            virtual Size nodeIndex(Size i, int j) const {
                return j - nodes_[i][0].j;
            }


        };

    }

}

#endif
