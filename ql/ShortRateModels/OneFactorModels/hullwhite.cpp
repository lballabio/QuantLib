/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/ShortRateModels/OneFactorModels/hullwhite.hpp>
#include <ql/Lattices/trinomialtree.hpp>
#include <ql/PricingEngines/blackmodel.hpp>

namespace QuantLib {

    HullWhite::HullWhite(const Handle<YieldTermStructure>& termStructure,
                         Real a, Real sigma)
    : Vasicek(termStructure->forwardRate(0.0, 0.0, Continuous, NoFrequency),
                                         a, 0.0, sigma, 0.0),
      TermStructureConsistentModel(termStructure) {
        b_ = NullParameter();
        lambda_ = NullParameter();
        generateArguments();
    }

    boost::shared_ptr<Lattice> HullWhite::tree(const TimeGrid& grid) const {

        TermStructureFittingParameter phi(termStructure());

        boost::shared_ptr<ShortRateDynamics> numericDynamics(
                                             new Dynamics(phi, a(), sigma()));

        boost::shared_ptr<TrinomialTree> trinomial(
                         new TrinomialTree(numericDynamics->process(), grid));

        boost::shared_ptr<ShortRateTree> numericTree(
                         new ShortRateTree(trinomial, numericDynamics, grid));

        typedef TermStructureFittingParameter::NumericalImpl NumericalImpl;
        boost::shared_ptr<NumericalImpl> impl =
            boost::dynamic_pointer_cast<NumericalImpl>(phi.implementation());
        impl->reset();
        for (Size i=0; i<(grid.size() - 1); i++) {
            Real discountBond = termStructure()->discount(grid[i+1]);
            const Array& statePrices = numericTree->statePrices(i);
            Size size = numericTree->size(i);
            Time dt = numericTree->timeGrid().dt(i);
            Real dx = trinomial->dx(i);
            Real x = trinomial->underlying(i,0);
            Real value = 0.0;
            for (Size j=0; j<size; j++) {
                value += statePrices[j]*std::exp(-x*dt);
                x += dx;
            }
            value = std::log(value/discountBond)/dt;
            impl->set(grid[i], value);
        }
        return numericTree;
    }

    Real HullWhite::A(Time t, Time T) const {
        DiscountFactor discount1 = termStructure()->discount(t);
        DiscountFactor discount2 = termStructure()->discount(T);
        Rate forward = termStructure()->forwardRate(t, t,
                                                    Continuous, NoFrequency);
        Real temp = sigma()*B(t,T);
        Real value = B(t,T)*forward - 0.25*temp*temp*B(0.0,2.0*t);
        return std::exp(value)*discount2/discount1;
    }

    void HullWhite::generateArguments() {
        phi_ = FittingParameter(termStructure(), a(), sigma());
    }

    Real HullWhite::discountBondOption(Option::Type type, Real strike,
                                       Time maturity,
                                       Time bondMaturity) const {

        Real v = sigma()*B(maturity, bondMaturity)*
            std::sqrt(0.5*(1.0 - std::exp(-2.0*a()*maturity))/a());
        Real f = termStructure()->discount(bondMaturity);
        Real k = termStructure()->discount(maturity)*strike;

        Real w = (type==Option::Call)? 1.0 : -1.0;

        return BlackModel::formula(f, k, v, w);
    }

}
