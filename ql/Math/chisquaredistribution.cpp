
/*
 Copyright (C) 2000, 2001, 2002, 2003 Sadruddin Rejeb

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

#include <ql/Math/chisquaredistribution.hpp>
#include <ql/Math/gammadistribution.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    double ChiSquareDistribution::operator()(double x) const {
        return GammaDistribution(0.5*df_)(0.5*x);
    }

    double NonCentralChiSquareDistribution::operator()(double x) const {
        if (x <= 0.0)
            return 0.0;

        const double errmax = 1e-12;
        const int itrmax = 10000;
        double lam = 0.5*ncp_;

        double u = QL_EXP(-lam);
        double v = u;
        double x2 = 0.5*x;
        double f2 = 0.5*df_;
        double f_x_2n = df_ - x;

        double t = 0.0;
        if (f2*QL_EPSILON > 0.125 &&
            QL_FABS(x2-f2) < QL_SQRT(QL_EPSILON)*f2) {
            t = QL_EXP((1 - t)*(2 - t/(f2+1)))/QL_SQRT(2.0*M_PI*(f2 + 1.0));
        }
        else {
            t = QL_EXP(f2*QL_LOG(x2) - x2 - 
                       GammaFunction().logValue(f2 + 1));
        }

        double ans = v*t;

        bool flag = false;
        int n = 1;
        double f_2n = df_ + 2.0;
        f_x_2n += 2.0;

        double bound;
        for (;;) {
            if (f_x_2n > 0) {
                flag = true;
                goto L10;
            }
            for (;;) {
                u *= lam / n;
                v += u;
                t *= x / f_2n;
                ans += v*t;
                n++; 
                f_2n += 2.0;
                f_x_2n += 2.0;
                if (!flag && n <= itrmax)
                    break;
            L10:
                bound = t * x / f_x_2n;
                if (bound <= errmax || n > itrmax)
                    goto L_End;
            }
        }
    L_End:
        if (bound > errmax) QL_FAIL("Didn't converge");
        return (ans);

    }

}
