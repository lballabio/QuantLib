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
/*! \file extendedcoxingersollross.cpp
    \brief Extended Cox-Ingersoll-Ross model

    \fullpath
    ql/ShortRateModels/OneFactorModels/%extendedcoxingersollross.cpp
*/

// $Id$

#include <ql/ShortRateModels/OneFactorModels/extendedcoxingersollross.hpp>
#include <ql/Math/chisquaredistribution.hpp>

namespace QuantLib {

    namespace ShortRateModels {

        ExtendedCoxIngersollRoss::ExtendedCoxIngersollRoss(
            const RelinkableHandle<TermStructure>& termStructure,
            double theta, double k, double sigma, double x0)
        : CoxIngersollRoss(theta, k, sigma, x0),
          TermStructureConsistentModel(termStructure) {
            generateParameters();
        }

        Handle<Lattices::Tree> ExtendedCoxIngersollRoss::tree(
            const TimeGrid& grid) const {
            TermStructureFittingParameter phi(termStructure());
            Handle<Dynamics> numericDynamics(
                new Dynamics(phi, theta(), k(), sigma(), x0()));
            return Handle<Lattices::Tree>(
                new ShortRateTree(numericDynamics, phi.implementation(),
                                     grid, true));
        }

        double ExtendedCoxIngersollRoss::A(Time t, Time s) const {
            double pt = termStructure()->discount(t);
            double ps = termStructure()->discount(s);
            double value = CoxIngersollRoss::A(t,s)*QL_EXP(B(t,s)*phi_(t))*
                (ps*CoxIngersollRoss::A(0.0,t)*QL_EXP(-B(0.0,t)*x0()))/
                (pt*CoxIngersollRoss::A(0.0,s)*QL_EXP(-B(0.0,s)*x0()));
            return value;
        }

        double ExtendedCoxIngersollRoss::discountBondOption(
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

            double sigma2 = sigma()*sigma();
            double h = QL_SQRT(k()*k() + 2.0*sigma2);
            double r0 = termStructure()->forward(0.0);
            double b = B(t,s);

            double rho = 2.0*h/(sigma2*(QL_EXP(h*t) - 1.0));
            double psi = (k() + h)/sigma2;
 
            double df = 4.0*k()*theta()/sigma2;
            double ncps = 2.0*rho*rho*(r0-phi_(0.0))*QL_EXP(h*t)/(rho+psi+b);
            double ncpt = 2.0*rho*rho*(r0-phi_(0.0))*QL_EXP(h*t)/(rho+psi);

            Math::NonCentralChiSquareDistribution chis(df, ncps);
            Math::NonCentralChiSquareDistribution chit(df, ncpt);

            double k = strike*
                (discountT*CoxIngersollRoss::A(0.0,s)*QL_EXP(-B(0.0,s)*x0()))/
                (discountS*CoxIngersollRoss::A(0.0,t)*QL_EXP(-B(0.0,t)*x0()));

            double r = QL_LOG(CoxIngersollRoss::A(t,s)/k)/b; 
            std::cout << r << std::endl;
            double call = discountS*chis(2.0*r*(rho+psi+b)) -
                k*discountT*chit(2.0*r*(rho+psi));
            if (type == Option::Call)
                return call;
            else
                return call - discountS + strike*discountT;
        }

    }

}
