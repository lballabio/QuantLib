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
/*! \file hullwhite.cpp
    \brief Hull & White model

    \fullpath
    ql/ShortRateModels/OneFactorModels/%hullwhite.cpp
*/

// $Id$

#include "ql/ShortRateModels/OneFactorModels/hullwhite.hpp"
#include "ql/blackmodel.hpp"

namespace QuantLib {

    namespace ShortRateModels {

        HullWhite::HullWhite(
            const RelinkableHandle<TermStructure>& termStructure, 
            double a, double sigma)
        : Vasicek(termStructure->forward(0.0), a, 0.0, sigma), 
          TermStructureConsistentModel(termStructure) {
            parameters_[1] = NullParameter();
            generateParameters();
        }

        Handle<Lattices::Tree> HullWhite::tree(const TimeGrid& timeGrid) const {

            TermStructureFittingParameter phi(termStructure());

            Handle<ShortRateDynamics> numericDynamics(
                new Dynamics(phi, a(), sigma()));

            Handle<ShortRateTree> numericTree(
                new ShortRateTree(numericDynamics, timeGrid));

            Handle<TermStructureFittingParameter::NumericalImpl> impl = 
                phi.implementation();
            impl->reset();
            for (Size i=0; i<(timeGrid.size() - 1); i++) {
                double discountBond = termStructure()->discount(timeGrid[i+1]);
                const std::vector<double>& statePrices = 
                    numericTree->statePrices(i);
                Size size = numericTree->column(i).size();
                Handle<Lattices::TrinomialBranching> 
                    branching(numericTree->column(i).branching());
                double dt = numericTree->timeGrid().dt(i);
                double dx = numericTree->dx(i);
                double x = branching->jMin()*dx;
                double value = 0.0;
                for (Size j=0; j<size; j++) {
                    value += statePrices[j]*QL_EXP(-x*dt);
                    x += dx;
                }
                value = QL_LOG(value/discountBond)/dt;
                impl->set(timeGrid[i], value);
            }
            return numericTree;
        }

        double HullWhite::A(Time t, Time T) const {
            double discount1 = termStructure()->discount(t);
            double discount2 = termStructure()->discount(T);
            double forward = termStructure()->forward(t);
            double temp = sigma()*B(T-t);
            double value = B(T-t)*forward - 0.5*temp*temp*B(2.0*t);
            return QL_EXP(value)*discount2/discount1;
        }

        double HullWhite::B(Time t) const {
            return (1.0 - QL_EXP(-a()*t))/a();
        }

        void HullWhite::generateParameters() {
            phi_ = FittingParameter(termStructure(), a(), sigma());
        }

        double HullWhite::discountBond(Time t, Time T, Rate r) const {
            return A(t,T)*QL_EXP( - B(T-t)*r);
        }

        double HullWhite::discountBondOption(
            Option::Type type, double strike, 
            Time maturity, Time bondMaturity) const {

            double v = sigma()*B(bondMaturity - maturity)*
                       QL_SQRT(0.5*(1.0 - QL_EXP(-2.0*a()*maturity))/a());
            double f = termStructure()->discount(bondMaturity);
            double k = termStructure()->discount(maturity)*strike;

            double w = (type==Option::Call)? 1.0 : -1.0;

            return BlackModel::formula(f, k, v, w);
        }

    }

}
