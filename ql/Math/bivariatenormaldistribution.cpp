
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

#include <ql/Math/bivariatenormaldistribution.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    const Real BivariateCumulativeNormalDistribution::x_[] = { 0.24840615,
        0.39233107, 0.21141819, 0.03324666, 0.00082485334 };
    const Real BivariateCumulativeNormalDistribution::y_[] = { 0.10024215,
        0.48281397, 1.06094980, 1.77972940, 2.66976040000 };

    Real BivariateCumulativeNormalDistribution::operator()(Real a,
                                                           Real b) const {

        CumulativeNormalDistribution cumNormalDist;
        Real CumNormDistA = cumNormalDist(a);
        Real CumNormDistB = cumNormalDist(b);
        Real MaxCumNormDistAB = QL_MAX(CumNormDistA, CumNormDistB);
        Real MinCumNormDistAB = QL_MIN(CumNormDistA, CumNormDistB);

        if (1.0-MaxCumNormDistAB<1e-15)
            return MinCumNormDistAB;

        if (MinCumNormDistAB<1e-15)
            return MinCumNormDistAB;

        Real a1 = a / std::sqrt(2.0 * (1.0 - rho_*rho_));
        Real b1 = b / std::sqrt(2.0 * (1.0 - rho_*rho_));

        Real result=-1.0;

        if (a<=0.0 && b<=0 && rho_<=0) {
            Real sum=0.0;
            for (Size i=0; i<5; i++) {
                for (Size j=0;j<5; j++) {
                    sum += x_[i]*x_[j]*
                        std::exp(a1*(2.0*y_[i]-a1)+b1*(2.0*y_[j]-b1)
                                 +2.0*rho_*(y_[i]-a1)*(y_[j]-b1));
                }
            }
            result = std::sqrt(1.0 - rho_*rho_)/M_PI*sum;
        } else if (a<=0 && b>=0 && rho_>=0) {
            BivariateCumulativeNormalDistribution bivCumNormalDist(-rho_);
            result= CumNormDistA - bivCumNormalDist(a, -b);
        } else if (a>=0.0 && b<=0.0 && rho_>=0.0) {
            BivariateCumulativeNormalDistribution bivCumNormalDist(-rho_);
            result= CumNormDistB - bivCumNormalDist(-a, b);
        } else if (a>=0.0 && b>=0.0 && rho_<=0.0) {
            result= CumNormDistA + CumNormDistB -1.0 + (*this)(-a, -b);
        } else if (a*b*rho_>0.0) {
            Real rho1 = (rho_*a-b)*(a>0.0 ? 1.0: -1.0)/
                std::sqrt(a*a-2.0*rho_*a*b+b*b);
            BivariateCumulativeNormalDistribution bivCumNormalDist(rho1);

            Real rho2 = (rho_*b-a)*(b>0.0 ? 1.0: -1.0)/
                std::sqrt(a*a-2.0*rho_*a*b+b*b);
            BivariateCumulativeNormalDistribution CBND2(rho2);

            Real delta = (1.0-(a>0.0 ? 1.0: -1.0)*(b>0.0 ? 1.0: -1.0))/4.0;

            result= bivCumNormalDist(a, 0.0) + CBND2(b, 0.0) - delta;
        } else {
            QL_FAIL("case not handled");
        }

        return result;

    }

}
