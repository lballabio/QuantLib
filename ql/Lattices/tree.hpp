
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

/*! \file tree.hpp
    \brief Tree class

    \fullpath
    ql/Lattices/%tree.hpp
*/

// $Id$

#ifndef quantlib_lattices_tree_h
#define quantlib_lattices_tree_h

#include <ql/qldefines.hpp>
#include <ql/termstructure.hpp>
#include <ql/Lattices/node.hpp>
#include <ql/Lattices/timegrid.hpp>

#include <iostream>
#include <list>
#include <vector>

namespace QuantLib {

    namespace Lattices {

        class Tree {
          public:
            Tree() {}
            Tree(unsigned int n) 
            : n_(n), dx_(0) {
                nodes_.push_back(std::vector<Handle<Node> >());
                nodes_[0].push_back(Handle<Node>(new Node(n,0,0)));
            }
            virtual ~Tree() {}

            virtual Node& node(unsigned int i, int j) = 0;

            const TimeGrid& timeGrid() { return t_; }

            void rollback(unsigned int from, unsigned int to);

            //! Returns t_i
            Time t(unsigned int i) const { return t_[i]; }

            //! Returns \delta t_i = t_{i+1} - t_i
            Time dt(unsigned int i) const { return t_[i+1] - t_[i]; }

            //! Returns \delta x_i
            double dx(unsigned int i) const { return dx_[i]; }

            //! Returns jMin
            int jMin(unsigned int i) const { return nodes_[i].front()->j(); }

            //! Returns jMax
            int jMax(unsigned int i) const { return nodes_[i].back()->j(); }

          protected:
            unsigned int n_;
            TimeGrid t_;
            std::vector<double> dx_;
            std::vector<std::vector<Handle<Node> > > nodes_;

        };

    }

}

#endif
