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
/*! \file twofactormodel.cpp
    \brief Abstract two-factor interest rate model class

    \fullpath
    ql/ShortRateModels/%twofactormodel.cpp
*/

// $Id$

#include <ql/ShortRateModels/twofactormodel.hpp>

namespace QuantLib {

    namespace ShortRateModels {

        using namespace Lattices;

        TwoFactorModel::TwoFactorModel(Size nParameters) : Model(nParameters) {}

        Handle<Lattice> TwoFactorModel::tree(const TimeGrid& grid) const {
            Handle<ShortRateDynamics> dyn = dynamics();

            Handle<TrinomialTree> tree1(
                new TrinomialTree(dyn->xProcess(), grid));
            Handle<TrinomialTree> tree2(
                new TrinomialTree(dyn->yProcess(), grid));

            return Handle<Lattice>( 
                new TwoFactorModel::ShortRateTree(tree1, tree2, dyn));
        }

        TwoFactorModel::ShortRateTree::ShortRateTree(
            const Handle<TrinomialTree>& tree1,
            const Handle<TrinomialTree>& tree2,
            const Handle<ShortRateDynamics>& dynamics)
        : Lattice2D(tree1, tree2, dynamics->correlation()), dynamics_(dynamics) 
        {}

    }

}
