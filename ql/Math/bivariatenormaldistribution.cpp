
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file bivariatenormaldistribution.cpp
    \brief bivariate cumulative normal distribution
*/

#include <ql/Math/bivariatenormaldistribution.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    const double BivariateCumulativeNormalDistribution::x_[] = { 0.24840615,
        0.39233107, 0.21141819, 0.03324666, 0.00082485334 };
    const double BivariateCumulativeNormalDistribution::y_[] = { 0.10024215,
        0.48281397, 1.06094980, 1.77972940, 2.66976040000 };

    double BivariateCumulativeNormalDistribution::operator()(double a,
                                                             double b) const {

        double a1 = a / QL_SQRT(2.0 * (1.0 - rho_*rho_));
        double b1 = b / QL_SQRT(2.0 * (1.0 - rho_*rho_));

        double result=-1.0;
    
        if (a<=0.0 && b<=0 && rho_<=0) {
            double sum=0.0;
            for (Size i=0; i<5; i++) { 
                for (Size j=0;j<5; j++) {
                    sum += x_[i]*x_[j]*
                        QL_EXP(a1*(2.0*y_[i]-a1)+b1*(2.0*y_[j]-b1)
                               +2.0*rho_*(y_[i]-a1)*(y_[j]-b1));
                }
            }
            result= QL_SQRT(1.0 - rho_*rho_)/M_PI*sum;
        } else if (a<=0 && b>=0 && rho_>=0) {
            CumulativeNormalDistribution cumNormalDist;
            BivariateCumulativeNormalDistribution bivCumNormalDist(-rho_);
            result= cumNormalDist(a) - bivCumNormalDist(a, -b);
        } else if (a>=0.0 && b<=0.0 && rho_>=0.0) {
            CumulativeNormalDistribution cumNormalDist;
            BivariateCumulativeNormalDistribution bivCumNormalDist(-rho_);
            result= cumNormalDist(b) - bivCumNormalDist(-a, b);
        } else if (a>=0.0 && b>=0.0 && rho_<=0.0) {
            CumulativeNormalDistribution cumNormalDist;
            result= cumNormalDist(a) + cumNormalDist(b) -1.0 + (*this)(-a, -b);
        } else if (a*b*rho_>0.0) {
            double rho1 = (rho_*a-b)*(a>0.0 ? 1.0: -1.0)/
                QL_SQRT(a*a-2.0*rho_*a*b+b*b);
            BivariateCumulativeNormalDistribution bivCumNormalDist(rho1);

            double rho2 = (rho_*b-a)*(b>0.0 ? 1.0: -1.0)/
                QL_SQRT(a*a-2.0*rho_*a*b+b*b);
            BivariateCumulativeNormalDistribution CBND2(rho2);

            double delta = (1.0-(a>0.0 ? 1.0: -1.0)*(b>0.0 ? 1.0: -1.0))/4.0;

            result= bivCumNormalDist(a, 0.0) + CBND2(b, 0.0) - delta;
        } else {
            QL_FAIL("BivariateCumulativeNormalDistribution::operator() : "
                    "case not handled");
        }

        return result;

    }

}
