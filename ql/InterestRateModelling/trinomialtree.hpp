
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
#include <ql/InterestRateModelling/timefunction.hpp>
#include <ql/Lattices/tree.hpp>

#include <list>
#include <vector>

namespace QuantLib {

    namespace InterestRateModelling {

        class TrinomialTree : public Lattices::Tree {
          public:
            TrinomialTree() : Tree(3) {}
/*            TrinomialTree(
                const Handle<ShortRateProcess>& process,
                const RelinkableHandle<TermStructure>& termStructure,
                Handle<TimeFunction>& theta,
                Time dtMax,
                const std::list<Time>& times);*/

            TrinomialTree(
                const Handle<ShortRateProcess>& process,
                const Lattices::TimeGrid& timeGrid);

            void addLevel(const std::vector<int>& k);

            virtual Lattices::Node& node(unsigned int i, int j) {
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
