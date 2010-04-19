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

#include <ql/models/shortrate/onefactormodels/extendedcoxingersollross.hpp>
#include <ql/methods/lattices/trinomialtree.hpp>
#include <ql/math/distributions/chisquaredistribution.hpp>

namespace QuantLib {

    ExtendedCoxIngersollRoss::ExtendedCoxIngersollRoss(
                              const Handle<YieldTermStructure>& termStructure,
                              Real theta, Real k, Real sigma, Real x0)
    : CoxIngersollRoss(x0, theta, k, sigma),
      TermStructureConsistentModel(termStructure) {
        generateArguments();
    }

    boost::shared_ptr<Lattice> ExtendedCoxIngersollRoss::tree(
                                                 const TimeGrid& grid) const {
        TermStructureFittingParameter phi(termStructure());
        boost::shared_ptr<Dynamics> numericDynamics(
                              new Dynamics(phi, theta(), k(), sigma(), x0()));

        boost::shared_ptr<TrinomialTree> trinomial(
                   new TrinomialTree(numericDynamics->process(), grid, true));

        typedef TermStructureFittingParameter::NumericalImpl NumericalImpl;
        boost::shared_ptr<NumericalImpl> impl =
            boost::dynamic_pointer_cast<NumericalImpl>(phi.implementation());

        return boost::shared_ptr<Lattice>(
                   new ShortRateTree(trinomial, numericDynamics, impl, grid));
    }

    Real ExtendedCoxIngersollRoss::A(Time t, Time s) const {
        Real pt = termStructure()->discount(t);
        Real ps = termStructure()->discount(s);
        Real value = CoxIngersollRoss::A(t,s)*std::exp(B(t,s)*phi_(t))*
            (ps*CoxIngersollRoss::A(0.0,t)*std::exp(-B(0.0,t)*x0()))/
            (pt*CoxIngersollRoss::A(0.0,s)*std::exp(-B(0.0,s)*x0()));
        return value;
    }

    Real ExtendedCoxIngersollRoss::discountBondOption(Option::Type type,
                                                      Real strike,
                                                      Time t, Time s) const {

        QL_REQUIRE(strike>0.0, "strike must be positive");

        DiscountFactor discountT = termStructure()->discount(t);
        DiscountFactor discountS = termStructure()->discount(s);
        if (t < QL_EPSILON) {
            switch(type) {
              case Option::Call:
                return std::max<Real>(discountS - strike, 0.0);
              case Option::Put:
                return std::max<Real>(strike - discountS, 0.0);
              default: QL_FAIL("unsupported option type");
            }
        }

        Real sigma2 = sigma()*sigma();
        Real h = std::sqrt(k()*k() + 2.0*sigma2);
        Real r0 = termStructure()->forwardRate(0.0, 0.0,
                                               Continuous, NoFrequency);
        Real b = B(t,s);

        Real rho = 2.0*h/(sigma2*(std::exp(h*t) - 1.0));
        Real psi = (k() + h)/sigma2;

        Real df = 4.0*k()*theta()/sigma2;
        Real ncps = 2.0*rho*rho*(r0-phi_(0.0))*std::exp(h*t)/(rho+psi+b);
        Real ncpt = 2.0*rho*rho*(r0-phi_(0.0))*std::exp(h*t)/(rho+psi);

        NonCentralChiSquareDistribution chis(df, ncps);
        NonCentralChiSquareDistribution chit(df, ncpt);

        Real z = std::log(CoxIngersollRoss::A(t,s)/strike)/b;
        Real call = discountS*chis(2.0*z*(rho+psi+b)) -
            strike*discountT*chit(2.0*z*(rho+psi));
        if (type == Option::Call)
            return call;
        else
            return call - discountS + strike*discountT;
    }

}

