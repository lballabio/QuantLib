
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/ShortRateModels/twofactormodel.hpp>

namespace QuantLib {

    TwoFactorModel::TwoFactorModel(Size nArguments) 
    : ShortRateModel(nArguments) {}

    boost::shared_ptr<Lattice> 
    TwoFactorModel::tree(const TimeGrid& grid) const {
        boost::shared_ptr<ShortRateDynamics> dyn = dynamics();

        boost::shared_ptr<TrinomialTree> tree1(
                                    new TrinomialTree(dyn->xProcess(), grid));
        boost::shared_ptr<TrinomialTree> tree2(
                                    new TrinomialTree(dyn->yProcess(), grid));

        return boost::shared_ptr<Lattice>( 
                        new TwoFactorModel::ShortRateTree(tree1, tree2, dyn));
    }

    TwoFactorModel::ShortRateTree::ShortRateTree(
                         const boost::shared_ptr<TrinomialTree>& tree1,
                         const boost::shared_ptr<TrinomialTree>& tree2,
                         const boost::shared_ptr<ShortRateDynamics>& dynamics)
    : Lattice2D(tree1, tree2, dynamics->correlation()), dynamics_(dynamics) 
    {}

}
