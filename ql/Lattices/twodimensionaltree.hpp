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
/*! \file twodimensionaltree.hpp
    \brief Two-dimensional tree class

    \fullpath
    ql/Lattices/%twodimensionaltree.hpp
*/

// $Id$

#ifndef quantlib_lattices_two_dimensional_tree_h
#define quantlib_lattices_two_dimensional_tree_h

#include <ql/Lattices/trinomialtree.hpp>
#include <ql/Math/matrix.hpp>

namespace QuantLib {

    namespace Lattices {

        class TwoDimensionalBranching : public Branching {
          public:
            TwoDimensionalBranching(double correlation,
                                    const Handle<Branching>& branching1,
                                    const Handle<Branching>& branching2);

            Size descendant(Size index, Size branch) const;

            double probability(Size index, Size branch) const;

            const Handle<TrinomialBranching>& branching1() const {
                return branching1_;
            }
            const Handle<TrinomialBranching>& branching2() const {
                return branching2_;
            }

            Size modulo() const { return modulo_; }
            void setModulo(Size modulo) { modulo_ = modulo; }
            void setNextModulo(Size nextModulo) { nextModulo_ = nextModulo; }

          private:
            double rho_;
            Size modulo_, nextModulo_;
            Handle<TrinomialBranching> branching1_, branching2_;
            Math::Matrix m_;
        };

        class TwoDimensionalTree : public Tree {
          public:
            TwoDimensionalTree(const Handle<TrinomialTree>& tree1,
                               const Handle<TrinomialTree>& tree2,
                               double correlation);
        };

    }

}

#endif
