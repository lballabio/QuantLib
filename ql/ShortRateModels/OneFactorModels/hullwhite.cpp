
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

#include <ql/ShortRateModels/OneFactorModels/hullwhite.hpp>
#include <ql/Lattices/trinomialtree.hpp>
#include <ql/PricingEngines/blackmodel.hpp>

namespace QuantLib {

    HullWhite::HullWhite(const RelinkableHandle<TermStructure>& termStructure, 
                         double a, double sigma)
    : Vasicek(termStructure->instantaneousForward(0.0), a, 0.0, sigma), 
      TermStructureConsistentModel(termStructure) {
        arguments_[1] = NullParameter();
        generateArguments();
    }

    Handle<Lattice> HullWhite::tree(const TimeGrid& grid) const {

        TermStructureFittingParameter phi(termStructure());

        Handle<ShortRateDynamics> numericDynamics(
                                             new Dynamics(phi, a(), sigma()));

        Handle<TrinomialTree> trinomial(
                         new TrinomialTree(numericDynamics->process(), grid));

        Handle<ShortRateTree> numericTree(
                         new ShortRateTree(trinomial, numericDynamics, grid));

        typedef TermStructureFittingParameter::NumericalImpl NumericalImpl;
        #if defined(HAVE_BOOST)
        Handle<NumericalImpl> impl = 
            boost::dynamic_pointer_cast<NumericalImpl>(phi.implementation());
        #else
        Handle<NumericalImpl> impl = phi.implementation();
        #endif
        impl->reset();
        for (Size i=0; i<(grid.size() - 1); i++) {
            double discountBond = termStructure()->discount(grid[i+1]);
            const Array& statePrices = numericTree->statePrices(i);
            Size size = numericTree->size(i);
            double dt = numericTree->timeGrid().dt(i);
            double dx = trinomial->dx(i);
            double x = trinomial->underlying(i,0);
            double value = 0.0;
            for (Size j=0; j<size; j++) {
                value += statePrices[j]*QL_EXP(-x*dt);
                x += dx;
            }
            value = QL_LOG(value/discountBond)/dt;
            impl->set(grid[i], value);
        }
        return numericTree;
    }

    double HullWhite::A(Time t, Time T) const {
        double discount1 = termStructure()->discount(t);
        double discount2 = termStructure()->discount(T);
        double forward = termStructure()->instantaneousForward(t);
        double temp = sigma()*B(t,T);
        double value = B(t,T)*forward - 0.25*temp*temp*B(0.0,2.0*t);
        return QL_EXP(value)*discount2/discount1;
    }

    void HullWhite::generateArguments() {
        phi_ = FittingParameter(termStructure(), a(), sigma());
    }

    double HullWhite::discountBondOption(Option::Type type, double strike, 
                                         Time maturity, 
                                         Time bondMaturity) const {

        double v = sigma()*B(maturity, bondMaturity)*
            QL_SQRT(0.5*(1.0 - QL_EXP(-2.0*a()*maturity))/a());
        double f = termStructure()->discount(bondMaturity);
        double k = termStructure()->discount(maturity)*strike;

        double w = (type==Option::Call)? 1.0 : -1.0;

        return BlackModel::formula(f, k, v, w);
    }

}
