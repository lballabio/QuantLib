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
/*! \file node.hpp
    \brief Node class

    \fullpath
    ql/Lattices/%node.hpp
*/

// $Id$

#ifndef quantlib_lattices_node_h
#define quantlib_lattices_node_h

#include <ql/termstructure.hpp>

namespace QuantLib {

    namespace Lattices {

        class Node {
          public:
            Node(int j)
            : j_(j), statePrice_(0.0) {}

            int j() const { return j_; }

            virtual double probability(Size branch) const = 0;

            double& statePrice() { return statePrice_; }
            const double& statePrice() const { return statePrice_; }

          private:
            int j_;

            double statePrice_;
        };

    }

}

#endif
