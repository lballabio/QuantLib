
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
/*! \file trinomialtree.hpp
    \brief Trinomial tree class

    \fullpath
    ql/InterestRateModelling/%trinomialtree.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_trinomial_tree_h
#define quantlib_interest_rate_modelling_trinomial_tree_h


#include <ql/qldefines.hpp>
#include <ql/termstructure.hpp>
#include <ql/InterestRateModelling/shortrateprocess.hpp>
#include <ql/Lattices/tree.hpp>

#include <vector>

namespace QuantLib {

    namespace InterestRateModelling {

        class TrinomialTree : public Lattices::Tree {
          public:
            TrinomialTree() : Lattices::Tree(3) {}

            TrinomialTree(const Handle<ShortRateProcess>& process,
                          const Lattices::TimeGrid& timeGrid);

            void addLevel(const std::vector<int>& k);

            virtual Lattices::Node& node(Size i, int j) {
                int jMin = nodes_[i][0]->j();
                return *(nodes_[i][j-jMin]);
            }

            virtual const Lattices::Node& node(Size i, int j) const {
                int jMin = nodes_[i][0]->j();
                return *(nodes_[i][j-jMin]);
            }

          private:
            class PrivateFunction;
            friend class PrivateFunction;
        };

    }

}

#endif
