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
/*! \file coxingersollross.cpp
    \brief Cox-Ingersoll-Ross model

    \fullpath
    ql/ShortRateModels/OneFactorModels/%coxingersollross.cpp
*/

// $Id$

#include "ql/ShortRateModels/OneFactorModels/coxingersollross.hpp"
#include "ql/Math/chisquaredistribution.hpp"

namespace QuantLib {

    namespace ShortRateModels {

        using Optimization::Constraint;
        using Optimization::PositiveConstraint;

        class CoxIngersollRoss::VolatilityConstraint : public Constraint {
          public:
            class Implementation : public ConstraintImpl {
              public:
                Implementation(const Parameter& theta, const Parameter& k) 
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
            VolatilityConstraint(const Parameter& theta, const Parameter& k)
            : Constraint(Handle<ConstraintImpl>(new Implementation(theta, k)))
            {}
        };

        CoxIngersollRoss::CoxIngersollRoss(
            double theta, double k, double sigma, double r0) 
        : OneFactorModel(4), 
          theta_(parameters_[0]), k_(parameters_[1]), 
          sigma_(parameters_[2]), r0_(parameters_[3]) {
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

        double CoxIngersollRoss::A(Time t) const {
            double sigma2 = sigma()*sigma();
            double h = QL_SQRT(k()*k() + 2.0*sigma2);
            double numerator = 2.0*h*QL_EXP(0.5*(k()+h)*t);
            double denominator = 2.0*h + (k()+h)*(QL_EXP(t*h) - 1.0);
            double value = QL_LOG(numerator/denominator)*
                2.0*k()*theta()/sigma2;
            return QL_EXP(value);
        }

        double CoxIngersollRoss::B(Time t) const {
            double h = QL_SQRT(k()*k() + 2.0*sigma()*sigma());
            double temp = QL_EXP(t*h) - 1.0;
            double numerator = 2.0*temp;
            double denominator = 2.0*h + (k()+h)*temp;
            double value = numerator/denominator;
            return value;
        }

        double CoxIngersollRoss::discountBond(
            Time t, Time s, Rate r) const {
            return  A(s-t)*QL_EXP(-B(s-t)*r);
        }

        double CoxIngersollRoss::discountBondOption(
            Option::Type type, double strike, Time t, Time s) const {

            double discountT = discountBond(0.0, t, x0());
            double discountS = discountBond(0.0, s, x0());

            if (t < QL_EPSILON) {
                switch(type) {
                  case Option::Call: return QL_MAX(discountS - strike, 0.0);
                  case Option::Put:  return QL_MAX(strike - discountS, 0.0);
                  default: throw Error("unsupported option type");
                }
            }

            double sigma2 = sigma()*sigma();
            double h = QL_SQRT(k()*k() + 2.0*sigma2);
            double b = B(s-t);

            double rho = 2.0*h/(sigma2*(QL_EXP(h*t) - 1.0));
            double psi = (k() + h)/sigma2;
 
            double df = 4.0*k()*theta()/sigma2;
            double ncps = 2.0*rho*rho*x0()*QL_EXP(h*t)/(rho+psi+b);
            double ncpt = 2.0*rho*rho*x0()*QL_EXP(h*t)/(rho+psi);

            Math::NonCentralChiSquareDistribution chis(df, ncps);
            Math::NonCentralChiSquareDistribution chit(df, ncpt);

            double k = strike*(discountT*A(s)*QL_EXP(-B(s)*x0()))/
                              (discountS*A(t)*QL_EXP(-B(t)*x0()));

            double r = QL_LOG(A(s-t)/k)/b; 
            double call = discountS*chis(2.0*r*(rho+psi+b)) -
                        k*discountT*chit(2.0*r*(rho+psi));
            if (type == Option::Call)
                return call;
            else
                return call - discountS + strike*discountT;

            return 0.0;
        }

    }

}
