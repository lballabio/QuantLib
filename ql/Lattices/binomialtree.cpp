/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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
/*! \file binomialtree.cpp
    \brief Binomial tree class

    \fullpath
    ql/Lattices/%binomialtree.cpp
*/

// $Id$

#include "ql/Lattices/binomialtree.hpp"

namespace QuantLib {

    namespace Lattices {

        StandardBinomialTree::StandardBinomialTree(
            const Handle<DiffusionProcess>& process,
            Time end, Size steps) : BinomialTree(steps + 1) {

            double dt = end/steps;

            x0_ = process->x0();
            dx_ = QL_SQRT(process->variance(0.0, 0.0, dt));
        }

    }

}

