/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/shortrate/twofactormodel.hpp>
#include <ql/processes/stochasticprocessarray.hpp>

namespace QuantLib {

    TwoFactorModel::TwoFactorModel(Size nArguments)
    : ShortRateModel(nArguments) {}

    std::shared_ptr<Lattice>
    TwoFactorModel::tree(const TimeGrid& grid) const {
        std::shared_ptr<ShortRateDynamics> dyn = dynamics();

        std::shared_ptr<TrinomialTree> tree1(
                                    new TrinomialTree(dyn->xProcess(), grid));
        std::shared_ptr<TrinomialTree> tree2(
                                    new TrinomialTree(dyn->yProcess(), grid));

        return std::shared_ptr<Lattice>(
                        new TwoFactorModel::ShortRateTree(tree1, tree2, dyn));
    }

    TwoFactorModel::ShortRateTree::ShortRateTree(
                         const std::shared_ptr<TrinomialTree>& tree1,
                         const std::shared_ptr<TrinomialTree>& tree2,
                         const std::shared_ptr<ShortRateDynamics>& dynamics)
    : TreeLattice2D<TwoFactorModel::ShortRateTree,TrinomialTree>(
                                       tree1, tree2, dynamics->correlation()),
      dynamics_(dynamics) {}

    std::shared_ptr<StochasticProcess>
    TwoFactorModel::ShortRateDynamics::process() const {
        Matrix correlation(2,2);
        correlation[0][0] = correlation[1][1] = 1.0;
        correlation[0][1] = correlation[1][0] = correlation_;
        std::vector<std::shared_ptr<StochasticProcess1D> > processes(2);
        processes[0] = xProcess_;
        processes[1] = yProcess_;
        return std::shared_ptr<StochasticProcess>(
                           new StochasticProcessArray(processes,correlation));
    }

}
