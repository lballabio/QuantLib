
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

#include <ql/ShortRateModels/OneFactorModels/coxingersollross.hpp>
#include <ql/Lattices/trinomialtree.hpp>
#include <ql/Math/chisquaredistribution.hpp>

namespace QuantLib {

    class CoxIngersollRoss::VolatilityConstraint : public Constraint {
      private:
        class Impl : public Constraint::Impl {
          public:
            Impl(const Parameter& theta, const Parameter& k)
            : theta_(theta), k_(k) {}
            bool test(const Array& params) const {
                if (params[0] <= 0.0)
                    return false;
                if (params[0] >= std::sqrt(2.0*k_(0.0)*theta_(0.0)))
                    return false;
                return true;
            }

          private:
            const Parameter& theta_;
            const Parameter& k_;
        };
      public:
        VolatilityConstraint(const Parameter& theta, const Parameter& k)
        : Constraint(boost::shared_ptr<Constraint::Impl>(
                                 new VolatilityConstraint::Impl(theta, k))) {}
    };

    CoxIngersollRoss::CoxIngersollRoss(Rate r0, Real theta,
                                       Real k, Real sigma)
    : OneFactorAffineModel(4),
      theta_(arguments_[0]), k_(arguments_[1]),
      sigma_(arguments_[2]), r0_(arguments_[3]) {
        theta_ = ConstantParameter(theta, PositiveConstraint());
        k_ = ConstantParameter(k, PositiveConstraint());
        sigma_ = ConstantParameter(sigma, VolatilityConstraint(theta_, k_));
        r0_ = ConstantParameter(r0, PositiveConstraint());
    }

    boost::shared_ptr<OneFactorModel::ShortRateDynamics>
    CoxIngersollRoss::dynamics() const {
        return boost::shared_ptr<ShortRateDynamics>(
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

        NonCentralChiSquareDistribution chis(df, ncps);
        NonCentralChiSquareDistribution chit(df, ncpt);

        Real z = std::log(A(t,s)/strike)/b;
        Real call = discountS*chis(2.0*z*(rho+psi+b)) -
            strike*discountT*chit(2.0*z*(rho+psi));

        if (type == Option::Call)
            return call;
        else
            return call - discountS + strike*discountT;
    }

    boost::shared_ptr<Lattice>
    CoxIngersollRoss::tree(const TimeGrid& grid) const {
        boost::shared_ptr<Tree> trinomial(
                        new TrinomialTree(dynamics()->process(), grid, true));
        return boost::shared_ptr<Lattice>(
                              new ShortRateTree(trinomial, dynamics(), grid));
    }

}
