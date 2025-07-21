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

#include <ql/models/shortrate/onefactormodels/coxingersollross.hpp>
#include <ql/methods/lattices/trinomialtree.hpp>
#include <ql/math/distributions/chisquaredistribution.hpp>

namespace QuantLib {

    class CoxIngersollRoss::VolatilityConstraint : public Constraint {
      private:
        class Impl final : public Constraint::Impl {
            Real k_, theta_;
          public:
            Impl(Real k, Real theta) : k_(k), theta_(theta) {}
            bool test(const Array& params) const override {
                Real sigma = params[0];
                return (sigma > 0.0) && (sigma*sigma < 2.0*k_*theta_);
            }
        };
      public:
        VolatilityConstraint(Real k, Real theta)
        : Constraint(ext::shared_ptr<Constraint::Impl>(
                                 new VolatilityConstraint::Impl(k, theta))) {}
    };

    CoxIngersollRoss::CoxIngersollRoss(Rate r0, Real theta,
                                       Real k, Real sigma,
                                       bool withFellerConstraint)
    : OneFactorAffineModel(4),
      theta_(arguments_[0]), k_(arguments_[1]),
      sigma_(arguments_[2]), r0_(arguments_[3]) {
        theta_ = ConstantParameter(theta, PositiveConstraint());
        k_ = ConstantParameter(k, PositiveConstraint());
        if (withFellerConstraint)
            sigma_ = ConstantParameter(sigma, VolatilityConstraint(k,theta));
        else
            sigma_ = ConstantParameter(sigma, PositiveConstraint());
        r0_ = ConstantParameter(r0, PositiveConstraint());
    }

    ext::shared_ptr<OneFactorModel::ShortRateDynamics>
    CoxIngersollRoss::dynamics() const {
        return ext::shared_ptr<ShortRateDynamics>(
                                  new Dynamics(theta(), k() , sigma(), x0()));
    }

    Real CoxIngersollRoss::A(Time t, Time T) const {
        Real sigma2 = sigma()*sigma();
        Real h = std::sqrt(k()*k() + 2.0*sigma2);
        Real numerator = 2.0*h*std::exp(0.5*(k()+h)*(T-t));
        Real denominator = 2.0*h + (k()+h)*(std::exp((T-t)*h) - 1.0);
        Real value = std::log(numerator/denominator)*
            2.0*k()*theta()/sigma2;
        return std::exp(value);
    }

    Real CoxIngersollRoss::B(Time t, Time T) const {
        Real h = std::sqrt(k()*k() + 2.0*sigma()*sigma());
        Real temp = std::exp((T-t)*h) - 1.0;
        Real numerator = 2.0*temp;
        Real denominator = 2.0*h + (k()+h)*temp;
        Real value = numerator/denominator;
        return value;
    }

    Real CoxIngersollRoss::discountBondOption(Option::Type type,
                                              Real strike,
                                              Time t, Time s) const {

        QL_REQUIRE(strike>0.0, "strike must be positive");
        DiscountFactor discountT = discountBond(0.0, t, x0());
        DiscountFactor discountS = discountBond(0.0, s, x0());

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
        Real b = B(t,s);

        Real rho = 2.0*h/(sigma2*(std::exp(h*t) - 1.0));
        Real psi = (k() + h)/sigma2;

        Real df = 4.0*k()*theta()/sigma2;
        Real ncps = 2.0*rho*rho*x0()*std::exp(h*t)/(rho+psi+b);
        Real ncpt = 2.0*rho*rho*x0()*std::exp(h*t)/(rho+psi);

        NonCentralCumulativeChiSquareDistribution chis(df, ncps);
        NonCentralCumulativeChiSquareDistribution chit(df, ncpt);

        Real z = std::log(A(t,s)/strike)/b;
        Real call = discountS*chis(2.0*z*(rho+psi+b)) -
            strike*discountT*chit(2.0*z*(rho+psi));

        if (type == Option::Call)
            return call;
        else
            return call - discountS + strike*discountT;
    }

    ext::shared_ptr<Lattice>
    CoxIngersollRoss::tree(const TimeGrid& grid) const {
        ext::shared_ptr<TrinomialTree> trinomial(
                        new TrinomialTree(dynamics()->process(), grid, true));
        return ext::shared_ptr<Lattice>(
                              new ShortRateTree(trinomial, dynamics(), grid));
    }

}

