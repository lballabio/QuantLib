/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Michal Kaut

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

#include <ql/math/distributions/bivariatestudenttdistribution.hpp>

namespace QuantLib {

    namespace {

        Real epsilon = 1.0e-8;

        Real sign(Real val) {
            return val == 0.0 ? 0.0
                : (val < 0.0 ? -1.0 : 1.0);
        }

        /* unlike the atan2 function in C++ that gives results in
           [-pi,pi], this returns a value in [0, 2*pi]
        */
        Real arctan(Real x, Real y) {
            Real res = std::atan2(x, y);
            return res >= 0.0 ? res : res + 2 * M_PI;
        }

        // function x(m,h,k) defined on top of page 155
        Real f_x(Real m, Real h, Real k, Real rho) {
            Real unCor = 1 - rho*rho;
            Real sub = std::pow(h - rho * k, 2);
            Real denom = sub + unCor * (m + k*k);
            if (denom < epsilon)
                return 0.0; // limit case for rho = +/-1.0
            return sub / (sub + unCor * (m + k*k));
        }

        // this calculates the cdf
        Real P_n(Real h, Real k, Natural n, Real rho) {
            Real unCor = 1.0 - rho*rho;

            Real div = 4 * std::sqrt(n * M_PI);
            Real xHK = f_x(n, h, k, rho);
            Real xKH = f_x(n, k, h, rho);
            Real divH = 1 + h*h / n;
            Real divK = 1 + k*k / n;
            Real sgnHK = sign(h - rho * k);
            Real sgnKH = sign(k - rho * h);

            if (n % 2 == 0) { // n is even, equation (10)
                // first line of (10)
                Real res = arctan(std::sqrt(unCor), -rho) / M_TWOPI;

                // second line of (10)
                Real dgM = 2 * (1 - xHK);  // multiplier for dgj
                Real gjM = sgnHK * 2 / M_PI; // multiplier for g_j
                // initializations for j = 1:
                Real f_j = std::sqrt(M_PI / divK);
                Real g_j = 1 + gjM * arctan(std::sqrt(xHK), std::sqrt(1 - xHK));
                Real sum = f_j * g_j;
                if (n >= 4) {
                    // different formulas for j = 2:
                    f_j *= 0.5 / divK; // (2 - 1.5) / (Real) (2 - 1) / divK;
                    Real dgj = gjM * std::sqrt(xHK * (1 - xHK));
                    g_j += dgj;
                    sum += f_j * g_j;
                    // and then the loop for the rest of the j's:
                    for (Natural j = 3; j <= n / 2; ++j) {
                        f_j *= (j - 1.5) / (Real) (j - 1) / divK;
                        dgj *= (Real) (j - 2) / (2 * j - 3) * dgM;
                        g_j += dgj;
                        sum += f_j * g_j;
                    }
                }
                res += k / div * sum;

                // third line of (10)
                dgM = 2 * (1 - xKH);
                gjM = sgnKH * 2 / M_PI;
                // initializations for j = 1:
                f_j = std::sqrt(M_PI / divH);
                g_j = 1 + gjM * arctan(std::sqrt(xKH), std::sqrt(1 - xKH));
                sum = f_j * g_j;
                if (n >= 4) {
                    // different formulas for j = 2:
                    f_j *= 0.5 / divH; // (2 - 1.5) / (Real) (2 - 1) / divK;
                    Real dgj = gjM * std::sqrt(xKH * (1 - xKH));
                    g_j += dgj;
                    sum += f_j * g_j;
                    // and then the loop for the rest of the j's:
                    for (Natural j = 3; j <= n / 2; ++j) {
                        f_j *= (j - 1.5) / (Real) (j - 1) / divH;
                        dgj *= (Real) (j - 2) / (2 * j - 3) * dgM;
                        g_j += dgj;
                        sum += f_j * g_j;
                    }
                }
                res += h / div * sum;
                return res;

            } else { // n is odd, equation (11)
                // first line of (11)
                Real hk = h * k;
                Real hkcn = hk + rho * n;
                Real sqrtExpr = std::sqrt(h*h - 2 * rho * hk + k*k + n * unCor);
                Real res = arctan(std::sqrt(Real(n)) * (-(h + k) * hkcn - (hk - n) * sqrtExpr),
                                  (hk - n) * hkcn - n * (h + k) * sqrtExpr ) / M_TWOPI;

                if (n > 1) {
                    // second line of (11)
                    Real mult = (1 - xHK) / 2;
                    // initializations for j = 1:
                    Real f_j = 2 / std::sqrt(M_PI) / divK;
                    Real dgj = sgnHK * std::sqrt(xHK);
                    Real g_j = 1 + dgj;
                    Real sum = f_j * g_j;
                    // and then the loop for the rest of the j's:
                    for (Natural j = 2; j <= (n - 1) / 2; ++j) {
                        f_j *= (Real) (j - 1) / (j - 0.5) / divK;
                        dgj *= (Real) (2 * j - 3) / (j - 1) * mult;
                        g_j += dgj;
                        sum += f_j * g_j;
                    }
                    res += k / div * sum;

                    // third line of (11)
                    mult = (1 - xKH) / 2;
                    // initializations for j = 1:
                    f_j = 2 / std::sqrt(M_PI) / divH;
                    dgj = sgnKH * std::sqrt(xKH);
                    g_j = 1 + dgj;
                    sum = f_j * g_j;
                    // and then the loop for the rest of the j's:
                    for (Natural j = 2; j <= (n - 1) / 2; ++j) {
                        f_j *= (Real) (j - 1) / (j - 0.5) / divH;
                        dgj *= (Real) (2 * j - 3) / (j - 1) * mult;
                        g_j += dgj;
                        sum += f_j * g_j;
                    }
                    res += h / div * sum;
                }
                return res;
            }
        }

    }


    BivariateCumulativeStudentDistribution::
    BivariateCumulativeStudentDistribution(Natural n,
                                           Real rho)
    : n_(n), rho_(rho) {}

    Real BivariateCumulativeStudentDistribution::operator()(Real x,
                                                            Real y) const {
        return P_n(x, y, n_, rho_);
    }

}
