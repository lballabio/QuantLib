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
    ql/InterestRateModelling/OneFactorModels/%coxingersollross.cpp
*/

// $Id$

#include "ql/InterestRateModelling/OneFactorModels/coxingersollross.hpp"
//#include "ql/Math/chisquaredistribution.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        using Optimization::Constraint;
        class CoxIngersollRoss::OwnConstraint : public Constraint {
            virtual bool test(const Array& params) const {
                if (params[0]<=0.0)
                    return false;
                if (params[1]<=0.0)
                    return false;
                if (params[2]<=0.0)
                    return false;
                if (params[2]*params[2]>=2.0*params[0]*params[1])
                    return false;
                return true;
            }
            virtual void correct(Array& params) const {
                params[0] = QL_MAX(params[2], 0.000000001);
                params[1] = QL_MAX(params[2], 0.000000001);
                params[2] = QL_MAX(params[2], 0.000000001);
                //FIXME
            }
        };

        class CoxIngersollRoss::Process : public ShortRateProcess {
          public:
            Process(CoxIngersollRoss * model)
            : model_(model), k_(model->k_),
              theta_(model->theta_), sigma_(model->sigma_), x0_(model->x0_) {}

            virtual double variable(Time t, Rate r) const {
                return QL_SQRT(r - model_->phi(t)) - QL_SQRT(x0_);
            }
            virtual double shortRate(Time t, double y) const {
                double v = y + QL_SQRT(x0_);
                return v*v + model_->phi(t);
            }
            virtual double drift(Time t, double y) const {
                double v = y + QL_SQRT(x0_);
                return (0.5*theta_*k_ - 0.125*sigma_*sigma_)/v - 0.5*k_*v;
            }
            virtual double diffusion(Time t, double y) const {
                return 0.5*sigma_;
            }
          private:
            CoxIngersollRoss * model_;
            const double& k_;
            const double& theta_;
            const double& sigma_;
            const double& x0_;
        };

        CoxIngersollRoss::CoxIngersollRoss(
            const RelinkableHandle<TermStructure>& termStructure)
        : OneFactorModel(3, termStructure), k_(params_[0]),
          theta_(params_[1]), sigma_(params_[2]), 
          x0_(termStructure->forward(0.0)) {
            k_ = 0.394529;
            theta_ = 0.271373;
            sigma_ = 0.0545128;
            process_ = Handle<ShortRateProcess>(new Process(this));
        }

        double CoxIngersollRoss::phi(Time t) const {
            double forwardRate = termStructure()->forward(t);
            double h = QL_SQRT(k_*k_ + 2.0*sigma_*sigma_);
            double expth = QL_EXP(t*h);
            //FIXME
            //Check this term
            //Not the same in paper and in book
            double temp = 2.0*h + (k_+h)*(expth-1.0);
            double phi = forwardRate - 2.0*k_*theta_*(expth - 1.0)/temp 
                - x0_*4.0*h*h*expth/(temp*temp);
            return phi;
        }

        double CoxIngersollRoss::A(Time t, Time T) const {
            double h = QL_SQRT(k_*k_ + 2.0*sigma_*sigma_);
            double numerator = 2.0*h*QL_EXP(0.5*(k_+h)*(T-t));
            double denominator = 2.0*h + (k_+h)*(QL_EXP((T - t)*h) - 1.0);
            double value = QL_LOG(numerator/denominator)*
                2.0*k_*theta_/(sigma_*sigma_);
            return QL_EXP(value);
        }

        double CoxIngersollRoss::B(Time t, Time T) const {
            double h = QL_SQRT(k_*k_ + 2.0*sigma_*sigma_);
            double numerator = 2.0*(QL_EXP((T-t)*h) - 1.0);
            double denominator = 2.0*h + (k_+h)*(QL_EXP((T - t)*h) - 1.0);
            double value = numerator/denominator;
            return value;
        }

        double CoxIngersollRoss::C(Time t, Time T) const {
            double Pt = termStructure()->discount(t);
            double PT = termStructure()->discount(T);
            double value = A(t,T)*QL_EXP(B(t,T)*phi(t))*
                (PT*A(0,t)*QL_EXP(-B(0,t)*x0_))/
                (Pt*A(0,T)*QL_EXP(-B(0,T)*x0_));
            return value;
        }

        double CoxIngersollRoss::discountBond(Time t, Time T, Rate r) const {
            double value =  C(t,T)*QL_EXP(-B(t,T)*r);
            return value;
        }
/*
        double CoxIngersollRoss::discountBondOption(
            Option::Type type, double strike, Time t, Time s) const {

            double discountT = termStructure()->discount(t);
            double discountS = termStructure()->discount(s);
            if (t < QL_EPSILON) {
                switch(type) {
                  case Option::Call: return QL_MAX(discountS - strike, 0.0);
                  case Option::Put:  return QL_MAX(strike - discountS, 0.0);
                  default: throw Error("unsupported option type");
                }
            }

            double sigma2 = sigma_*sigma_;
            double h = QL_SQRT(k_*k_ + 2.0*sigma2);
            double r0 = termStructure()->forward(0.0);
            double b = B(t,s);

            double rho = 2.0*h/(sigma2*(QL_EXP(h*t) - 1.0));
            double psi = (k_ + h)/sigma2;
 
            double df = 4.0*k_*theta_/sigma2;
            double ncps = 2.0*rho*rho*(r0-phi(0.0))*QL_EXP(h*t)/(rho+psi+b);
            double ncpt = 2.0*rho*rho*(r0-phi(0.0))*QL_EXP(h*t)/(rho+psi);

            Math::NonCentricChiSquareDistribution chis(df, ncps);
            Math::NonCentricChiSquareDistribution chit(df, ncpt);

            double k = strike*
                (discountT*A(0,s)*QL_EXP(-B(0,s)*x0_))/
                (discountS*A(0,t)*QL_EXP(-B(0,t)*x0_));

            double r = QL_LOG(A(t,s)/k)/b; 
            std::cout << r << std::endl;
            double call = discountS*chis(2.0*r*(rho+psi+b)) -
                k*discountT*chit(2.0*r*(rho+psi));
            if (type == Option::Call)
                return call;
            else
                return call - discountS + strike*discountT;
        }
*/
    }

}
