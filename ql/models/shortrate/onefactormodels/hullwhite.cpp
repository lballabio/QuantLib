/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 Chiara Fornarola
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/methods/lattices/trinomialtree.hpp>
#include <ql/pricingengines/blackformula.hpp>

using std::exp;
using std::sqrt;

namespace QuantLib {

    HullWhite::HullWhite(const Handle<YieldTermStructure>& termStructure,
                         Real a, Real sigma)
    : Vasicek(termStructure->forwardRate(0.0, 0.0, Continuous, NoFrequency),
                                         a, 0.0, sigma, 0.0),
      TermStructureConsistentModel(termStructure) {
        b_ = NullParameter();
        lambda_ = NullParameter();
        generateArguments();

        registerWith(termStructure);
    }

    ext::shared_ptr<Lattice> HullWhite::tree(const TimeGrid& grid) const {

        TermStructureFittingParameter phi(termStructure());
        ext::shared_ptr<ShortRateDynamics> numericDynamics(
                                             new Dynamics(phi, a(), sigma()));
        ext::shared_ptr<TrinomialTree> trinomial(
                         new TrinomialTree(numericDynamics->process(), grid));
        ext::shared_ptr<ShortRateTree> numericTree(
                         new ShortRateTree(trinomial, numericDynamics, grid));

        typedef TermStructureFittingParameter::NumericalImpl NumericalImpl;
        ext::shared_ptr<NumericalImpl> impl =
            ext::dynamic_pointer_cast<NumericalImpl>(phi.implementation());
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

        Real _a = a();
        Real v;
        if (_a < std::sqrt(QL_EPSILON)) {
            v = sigma()*B(maturity, bondMaturity)* std::sqrt(maturity);
        } else {
            v = sigma()*B(maturity, bondMaturity)*
                std::sqrt(0.5*(1.0 - std::exp(-2.0*_a*maturity))/_a);
        }
        Real f = termStructure()->discount(bondMaturity);
        Real k = termStructure()->discount(maturity)*strike;

        return blackFormula(type, k, f, v);
    }

    Real HullWhite::discountBondOption(Option::Type type, Real strike,
                                       Time maturity, Time bondStart,
                                       Time bondMaturity) const {

        Real _a = a();
        Real v;
        if (_a < std::sqrt(QL_EPSILON)) {
            v = sigma()*B(bondStart, bondMaturity)* std::sqrt(maturity);
        } else {
            Real c = exp(-2.0*_a*(bondStart-maturity))
                - exp(-2.0*_a*bondStart)
                -2.0*(exp(-_a*(bondStart+bondMaturity-2.0*maturity))
                      - exp(-_a*(bondStart+bondMaturity)))
                + exp(-2.0*_a*(bondMaturity-maturity))
                - exp(-2.0*_a*bondMaturity);
            // The above should always be positive, but due to
            // numerical errors it can be a very small negative number.
            // We floor it at 0 to avoid NaNs.
            v = sigma()/(_a*sqrt(2.0*_a)) * sqrt(std::max(c, 0.0));
        }
        Real f = termStructure()->discount(bondMaturity);
        Real k = termStructure()->discount(bondStart)*strike;

        return blackFormula(type, k, f, v);
    }

    Rate HullWhite::convexityBias(Real futuresPrice,
                                  Time t,
                                  Time T,
                                  Real sigma,
                                  Real a) {
        QL_REQUIRE(futuresPrice>=0.0,
            "negative futures price (" << futuresPrice << ") not allowed");
        QL_REQUIRE(t>=0.0,
            "negative t (" << t << ") not allowed");
        QL_REQUIRE(T>=t,
            "T (" << T << ") must not be less than t (" << t << ")");
        QL_REQUIRE(sigma>=0.0,
            "negative sigma (" << sigma << ") not allowed");
        QL_REQUIRE(a>=0.0,
            "negative a (" << a << ") not allowed");

        Time deltaT = (T-t);
        Real tempDeltaT = (1.-std::exp(-a*deltaT)) / a;
        Real halfSigmaSquare = sigma*sigma/2.0;

        // lambda adjusts for the fact that the underlying is an interest rate
        Real lambda = halfSigmaSquare * (1.-std::exp(-2.0*a*t)) / a *
            tempDeltaT * tempDeltaT;

        Real tempT = (1.0 - std::exp(-a*t)) / a;

        // phi is the MtM adjustment
        Real phi = halfSigmaSquare * tempDeltaT * tempT * tempT;

        // the adjustment
        Real z = lambda + phi;

        Rate futureRate = (100.0-futuresPrice)/100.0;
        return (1.0-std::exp(-z)) * (futureRate + 1.0/(T-t));
    }

}

