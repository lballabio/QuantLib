
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

/*! \file tree.cpp
    \brief Tree class

    \fullpath
    ql/Lattices/%tree.cpp
*/

// $Id$

#include <iostream>

#include "ql/Lattices/tree.hpp"

namespace QuantLib {

    namespace Lattices {

        using std::cout;
        using std::endl;

        void Tree::rollback(unsigned int from, unsigned int to) {
            QL_REQUIRE(from>=to, "Wrong rollback extremities");

            for (int i=(int)(from-1); i>=(int)to; i--) {
                for (int j=jMin(i); j<=jMax(i); j++) {
                    double value = 0.0;
                    for (unsigned k=0; k<n_; k++) {
                        value += node(i,j).probability(k)
                            *node(i,j).descendant(k).value();
                    }
                    node(i,j).setValue(value*node(i,j).discount());
                }
            }
        }

    }

}

