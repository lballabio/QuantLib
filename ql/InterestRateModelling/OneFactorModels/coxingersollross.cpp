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

        class ExtendedCoxIngersollRoss::OwnConstraint : public Constraint {
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

        ExtendedCoxIngersollRoss::ExtendedCoxIngersollRoss(
            const RelinkableHandle<TermStructure>& termStructure) 
        : GeneralCoxIngersollRoss(ConstantParameter(0.271373), 
                                  ConstantParameter(0.394529), 
                                  ConstantParameter(0.0545128), 
                                  termStructure) {
            constraint_ = Handle<Constraint>(new OwnConstraint());
            generateParameters();
        }

        double ExtendedCoxIngersollRoss::A(Time t) const {
            double sigma2 = sigma()*sigma();
            double h = QL_SQRT(k()*k() + 2.0*sigma2);
            double numerator = 2.0*h*QL_EXP(0.5*(k()+h)*t);
            double denominator = 2.0*h + (k()+h)*(QL_EXP(t*h) - 1.0);
            double value = QL_LOG(numerator/denominator)*
                2.0*k()*theta()/sigma2;
            return QL_EXP(value);
        }

        double ExtendedCoxIngersollRoss::B(Time t) const {
            double h = QL_SQRT(k()*k() + 2.0*sigma()*sigma());
            double temp = QL_EXP(t*h) - 1.0;
            double numerator = 2.0*temp;
            double denominator = 2.0*h + (k()+h)*temp;
            double value = numerator/denominator;
            return value;
        }

        double ExtendedCoxIngersollRoss::C(Time t, Time s) const {
            double pt = termStructure()->discount(t);
            double ps = termStructure()->discount(s);
            double value = A(s-t)*QL_EXP(B(s-t)*phi_(t))*
                (ps*A(t)*QL_EXP(-B(t)*x0_))/
                (pt*A(s)*QL_EXP(-B(s)*x0_));
            return value;
        }

        double ExtendedCoxIngersollRoss::discountBond(
            Time t, Time s, Rate r) const {
            double value =  C(t,s)*QL_EXP(-B(s-t)*r);
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
