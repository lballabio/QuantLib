
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
                if (params[0] >= QL_SQRT(2.0*k_(0.0)*theta_(0.0)))
                    return false;
                return true;
            }

          private:
            const Parameter& theta_;
            const Parameter& k_;
        };
      public:
        VolatilityConstraint(const Parameter& theta, const Parameter& k)
        : Constraint(Handle<Constraint::Impl>(
                                 new VolatilityConstraint::Impl(theta, k))) {}
    };

    CoxIngersollRoss::CoxIngersollRoss(Rate r0, double theta, 
                                       double k, double sigma) 
    : OneFactorAffineModel(4), 
      theta_(arguments_[0]), k_(arguments_[1]), 
      sigma_(arguments_[2]), r0_(arguments_[3]) {
        theta_ = ConstantParameter(theta, PositiveConstraint());
        k_ = ConstantParameter(k, PositiveConstraint());
        sigma_ = ConstantParameter(sigma, VolatilityConstraint(theta_, k_));
        r0_ = ConstantParameter(r0, PositiveConstraint());
    }

    Handle<OneFactorModel::ShortRateDynamics> 
    CoxIngersollRoss::dynamics() const {
        return Handle<ShortRateDynamics>(
                                  new Dynamics(theta(), k() , sigma(), x0()));
    }

    double CoxIngersollRoss::A(Time t, Time T) const {
        double sigma2 = sigma()*sigma();
        double h = QL_SQRT(k()*k() + 2.0*sigma2);
        double numerator = 2.0*h*QL_EXP(0.5*(k()+h)*(T-t));
        double denominator = 2.0*h + (k()+h)*(QL_EXP((T-t)*h) - 1.0);
        double value = QL_LOG(numerator/denominator)*
            2.0*k()*theta()/sigma2;
        return QL_EXP(value);
    }

    double CoxIngersollRoss::B(Time t, Time T) const {
        double h = QL_SQRT(k()*k() + 2.0*sigma()*sigma());
        double temp = QL_EXP((T-t)*h) - 1.0;
        double numerator = 2.0*temp;
        double denominator = 2.0*h + (k()+h)*temp;
        double value = numerator/denominator;
        return value;
    }

    double CoxIngersollRoss::discountBondOption(Option::Type type, 
                                                double strike, 
                                                Time t, Time s) const {

        QL_REQUIRE(strike>0.0,
            "CoxIngersollRoss::discountBondOption :"
            "strike must be positive");
        double discountT = discountBond(0.0, t, x0());
        double discountS = discountBond(0.0, s, x0());

        if (t < QL_EPSILON) {
            switch(type) {
              case Option::Call: return QL_MAX(discountS - strike, 0.0);
              case Option::Put:  return QL_MAX(strike - discountS, 0.0);
              default: QL_FAIL("unsupported option type");
            }
        }

        double sigma2 = sigma()*sigma();
        double h = QL_SQRT(k()*k() + 2.0*sigma2);
        double b = B(t,s);

        double rho = 2.0*h/(sigma2*(QL_EXP(h*t) - 1.0));
        double psi = (k() + h)/sigma2;

        double df = 4.0*k()*theta()/sigma2;
        double ncps = 2.0*rho*rho*x0()*QL_EXP(h*t)/(rho+psi+b);
        double ncpt = 2.0*rho*rho*x0()*QL_EXP(h*t)/(rho+psi);

        NonCentralChiSquareDistribution chis(df, ncps);
        NonCentralChiSquareDistribution chit(df, ncpt);

        double z = QL_LOG(A(t,s)/strike)/b; 
        double call = discountS*chis(2.0*z*(rho+psi+b)) -
            strike*discountT*chit(2.0*z*(rho+psi));

        if (type == Option::Call)
            return call;
        else
            return call - discountS + strike*discountT;
    }

    Handle<Lattice> CoxIngersollRoss::tree(const TimeGrid& grid) const {
        Handle<Tree> trinomial(
                        new TrinomialTree(dynamics()->process(), grid, true));
        return Handle<Lattice>(new ShortRateTree(trinomial, dynamics(), grid));
    }

}
