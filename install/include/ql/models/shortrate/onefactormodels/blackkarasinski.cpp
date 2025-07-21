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

#include <ql/models/shortrate/onefactormodels/blackkarasinski.hpp>
#include <ql/methods/lattices/trinomialtree.hpp>
#include <ql/math/solvers1d/brent.hpp>

namespace QuantLib {

    // Private function used by solver to determine time-dependent parameter
    class BlackKarasinski::Helper {
      public:
        Helper(Size i, Real xMin, Real dx,
               Real discountBondPrice,
               const ext::shared_ptr<ShortRateTree>& tree)
        : size_(tree->size(i)),
          dt_(tree->timeGrid().dt(i)),
          xMin_(xMin), dx_(dx),
          statePrices_(tree->statePrices(i)),
          discountBondPrice_(discountBondPrice) {}

        Real operator()(Real theta) const {
            Real value = discountBondPrice_;
            Real x = xMin_;
            for (Size j=0; j<size_; j++) {
                Real discount = std::exp(-std::exp(theta+x)*dt_);
                value -= statePrices_[j]*discount;
                x += dx_;
            }
            return value;
        }

      private:
        Size size_;
        Time dt_;
        Real xMin_, dx_;
        const Array& statePrices_;
        Real discountBondPrice_;
    };

    BlackKarasinski::BlackKarasinski(
                              const Handle<YieldTermStructure>& termStructure,
                              Real a, Real sigma)
    : OneFactorModel(2), TermStructureConsistentModel(termStructure),
      a_(arguments_[0]), sigma_(arguments_[1]) {
        a_ = ConstantParameter(a, PositiveConstraint());
        sigma_ = ConstantParameter(sigma, PositiveConstraint());
        phi_ = TermStructureFittingParameter(termStructure);

        registerWith(termStructure);
    }

    ext::shared_ptr<Lattice>
    BlackKarasinski::tree(const TimeGrid& grid) const {

        ext::shared_ptr<ShortRateDynamics> numericDynamics(
                         new Dynamics(phi_, a(), sigma()));
        ext::shared_ptr<TrinomialTree> trinomial(
                         new TrinomialTree(numericDynamics->process(), grid));
        ext::shared_ptr<ShortRateTree> numericTree(
                         new ShortRateTree(trinomial, numericDynamics, grid));

        typedef TermStructureFittingParameter::NumericalImpl NumericalImpl;
        ext::shared_ptr<NumericalImpl> impl =
            ext::dynamic_pointer_cast<NumericalImpl>(phi_.implementation());
        impl->reset();
        Real value = 1.0;
        Real vMin = -50.0;
        Real vMax = 50.0;
        for (Size i=0; i<(grid.size() - 1); i++) {
            Real discountBond = termStructure()->discount(grid[i+1]);
            Real xMin = trinomial->underlying(i, 0);
            Real dx = trinomial->dx(i);
            Helper finder(i, xMin, dx, discountBond, numericTree);
            Brent s1d;
            s1d.setMaxEvaluations(1000);
            value = s1d.solve(finder, 1e-7, value, vMin, vMax);
            impl->set(grid[i], value);
        }
        return numericTree;
    }

    ext::shared_ptr<OneFactorModel::ShortRateDynamics>
        BlackKarasinski::dynamics() const {
        // Calibrate fitting parameter to term structure
        Size steps = 50;
        ext::shared_ptr<Lattice> lattice = this->tree(
            TimeGrid(termStructure()->maxTime(), steps));
        ext::shared_ptr<ShortRateDynamics> numericDynamics(
            new Dynamics(phi_, a(), sigma()));
        return numericDynamics;
    }

}
