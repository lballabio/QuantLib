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
/*! \file binomialtree.hpp
    \brief Binomial tree class

    \fullpath
    ql/Lattices/%binomialtree.hpp
*/

// $Id$

#ifndef quantlib_lattices_binomial_tree_h
#define quantlib_lattices_binomial_tree_h

#include <ql/diffusionprocess.hpp>
#include <ql/Lattices/tree.hpp>

namespace QuantLib {

    namespace Lattices {

        //! Binomial tree base class
        class BinomialTree : public Tree {
          public:
            BinomialTree(Size nColumns) : Tree(nColumns) {}

            Size size(Size i) const {
                return i+1;
            }
            Size descendant(Size i, Size index, Size branch) const {
                return index + branch;
            }
        };

        //! Additive binomial tree class with equal probabilities
        class StandardBinomialTree : public BinomialTree {
          public:
            StandardBinomialTree(const Handle<DiffusionProcess>& process,
                                 Time end, Size steps);

            double probability(Size i, Size index, Size branch) const {
                return 0.5;
            }
            double underlying(Size i, Size index) const {
                int j = (2*index - i);
                return x0_ + j*dx_;
            }
          private:
            double x0_, dx_;

        };

    }

}

#endif
