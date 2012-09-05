/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

#include <ql/models/shortrate/onefactormodel.hpp>
#include <ql/stochasticprocess.hpp>
#include <ql/math/solvers1d/brent.hpp>

namespace QuantLib {

    //Private function used by solver to determine time-dependent parameter
    class OneFactorModel::ShortRateTree::Helper {
      public:
        Helper(Size i,
               Real discountBondPrice,
               const boost::shared_ptr
                   <TermStructureFittingParameter::NumericalImpl>& theta,
               ShortRateTree& tree)
        : size_(tree.size(i)),
          i_(i),
          statePrices_(tree.statePrices(i)),
          discountBondPrice_(discountBondPrice),
          theta_(theta),
          tree_(tree) {
            theta_->set(tree.timeGrid()[i], 0.0);
        }

        Real operator()(Real theta) const {
            Real value = discountBondPrice_;
            theta_->change(theta);
            for (Size j=0; j<size_; j++)
                value -= statePrices_[j]*tree_.discount(i_,j);
            return value;
        }

      private:
        Size size_;
        Size i_;
        const Array& statePrices_;
        Real discountBondPrice_;
        boost::shared_ptr<TermStructureFittingParameter::NumericalImpl> theta_;
        ShortRateTree& tree_;
    };

    OneFactorModel::ShortRateTree::ShortRateTree(
            const boost::shared_ptr<TrinomialTree>& tree,
            const boost::shared_ptr<ShortRateDynamics>& dynamics,
            const boost::shared_ptr
                <TermStructureFittingParameter::NumericalImpl>& theta,
            const TimeGrid& timeGrid)
    : TreeLattice1D<OneFactorModel::ShortRateTree>(timeGrid, tree->size(1)),
      tree_(tree), dynamics_(dynamics) {

        theta->reset();
        Real value = 1.0;
        Real vMin = -100.0;
        Real vMax = 100.0;
        for (Size i=0; i<(timeGrid.size() - 1); i++) {
            Real discountBond = theta->termStructure()->discount(t_[i+1]);
            Helper finder(i, discountBond, theta, *this);
            Brent s1d;
            s1d.setMaxEvaluations(1000);
            value = s1d.solve(finder, 1e-7, value, vMin, vMax);
            // vMin = value - 1.0;
            // vMax = value + 1.0;
            theta->change(value);
        }
    }

    OneFactorModel::ShortRateTree::ShortRateTree(
                         const boost::shared_ptr<TrinomialTree>& tree,
                         const boost::shared_ptr<ShortRateDynamics>& dynamics,
                         const TimeGrid& timeGrid)
    : TreeLattice1D<OneFactorModel::ShortRateTree>(timeGrid, tree->size(1)),
      tree_(tree), dynamics_(dynamics) {}

    OneFactorModel::OneFactorModel(Size nArguments)
    : ShortRateModel(nArguments) {}

    boost::shared_ptr<Lattice>
    OneFactorModel::tree(const TimeGrid& grid) const {
        boost::shared_ptr<TrinomialTree> trinomial(
                              new TrinomialTree(dynamics()->process(), grid));
        return boost::shared_ptr<Lattice>(
                              new ShortRateTree(trinomial, dynamics(), grid));
    }

    DiscountFactor OneFactorAffineModel::discount(Time t) const {
        Real x0 = dynamics()->process()->x0();
        Rate r0 = dynamics()->shortRate(0.0, x0);
        return discountBond(0.0, t, r0);
    }

}

