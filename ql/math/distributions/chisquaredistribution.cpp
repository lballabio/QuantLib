/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2007 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/distributions/chisquaredistribution.hpp>
#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <boost/math/special_functions/gamma.hpp>

namespace QuantLib {

    Real CumulativeChiSquareDistribution::operator()(Real x) const {
        return CumulativeGammaDistribution(0.5*df_)(0.5*x);
    }

    Real NonCentralCumulativeChiSquareDistribution::operator()(Real x) const {
        if (x <= 0.0)
            return 0.0;

        const Real errmax = 1e-12;
        const Size itrmax = 1000000;
        const Real minLog = std::log(QL_MIN_POSITIVE_REAL);
        Real lam = 0.5*ncp_;
        Real x2 = 0.5*x;
        Real f2 = 0.5*df_;

        // For large ncp or x, the Poisson weights u and the first terms t
        // of the series underflow to zero, and so would the whole sum.
        // We use their logarithms (which increase with n up to lam and
        // x2-f2, respectively) to find the first representable ones and
        // skip the corresponding (negligible) terms.
        const auto logU = [&](Size n) {
            return n*std::log(lam) - lam - boost::math::lgamma(n + 1.0);
        };
        const auto logT = [&](Size n) {
            return (f2 + n)*std::log(x2) - x2 - boost::math::lgamma(f2 + n + 1.0);
        };
        const Size never = std::numeric_limits<Size>::max();
        const auto firstT = [&](Size from) {
            Real peak = std::min(std::floor(x2 - f2), Real(from + itrmax));
            if (peak < from)
                return logT(from) >= minLog ? from : never;
            Size lo = from, hi = Size(peak);
            if (logT(hi) < minLog)
                return never;
            while (lo < hi) {
                Size mid = lo + (hi - lo)/2;
                if (logT(mid) >= minLog)
                    hi = mid;
                else
                    lo = mid + 1;
            }
            return lo;
        };

        bool uSmall = -lam < minLog;
        Real u = uSmall ? Real(0.0) : std::exp(-lam);
        Real v = u;

        Real t = 0.0;
        bool tSmall = false;
        if (f2*QL_EPSILON > 0.125 &&
            std::fabs(x2-f2) < std::sqrt(QL_EPSILON)*f2) {
            t = std::exp((1 - t) *
                         (2 - t/(f2+1)))/std::sqrt(2.0*M_PI*(f2 + 1.0));
        }
        else {
            Real lt = f2*std::log(x2) - x2 - GammaFunction().logValue(f2 + 1);
            tSmall = lt < minLog;
            if (!tSmall)
                t = std::exp(lt);
        }

        if (tSmall && x > df_ + ncp_) {
            // Chernoff bound on the upper tail.  If it's negligible,
            // return 1 instead of summing about x/2 terms to get there.
            Real w = (df_ + std::sqrt(df_*df_ + 4.0*x*ncp_)) / (2.0*x);
            Real logBound = -0.5*(1.0-w)*x - 0.5*df_*std::log(w)
                + 0.5*ncp_*(1.0-w)/w;
            if (logBound < std::log(errmax))
                return 1.0;
        }

        Real ans = v*t;

        Size n = 1;
        Size nT = tSmall ? firstT(n) : 0;
        if (uSmall) {
            // jump to the first representable Poisson weight
            QL_REQUIRE(lam < QL_MAX_INTEGER,
                       "non-centrality parameter too large");
            Size lo = n, hi = Size(lam);
            while (lo < hi) {
                Size mid = lo + (hi - lo)/2;
                if (logU(mid) >= minLog)
                    hi = mid;
                else
                    lo = mid + 1;
            }
            n = lo;
            u = v = std::exp(logU(n));
            Real lt = logT(n);
            tSmall = lt < minLog;
            t = tSmall ? Real(0.0) : std::exp(lt);
            nT = tSmall ? firstT(n + 1) : 0;
            ans = v*t;
            n++;
        }
        const Size nMax = n + itrmax;

        bool flag = false;
        Real f_2n = df_ + 2.0*n;
        Real f_x_2n = df_ - x + 2.0*n;

        Real bound;
        for (;;) {
            if (f_x_2n > 0) {
                flag = true;
                goto L10;
            }
            for (;;) {
                u *= lam / n;
                v += u;
                if (tSmall) {
                    if (n >= nT) {
                        t = std::exp(logT(n));
                        tSmall = false;
                    }
                } else {
                    t *= x / f_2n;
                }
                ans += v*t;
                n++;
                f_2n += 2.0;
                f_x_2n += 2.0;
                if (!flag && n <= nMax)
                    break;
            L10:
                bound = t * x / f_x_2n;
                if (bound <= errmax || n > nMax)
                    goto L_End;
            }
        }
    L_End:
        // f_x_2n <= 0 means that we ran out of iterations before
        // the error bound could even be computed
        if (f_x_2n <= 0 || bound > errmax) QL_FAIL("didn't converge");
        return (ans);

    }

    Real NonCentralCumulativeChiSquareSankaranApprox::operator()(Real x) const {

        const Real h = 1-2*(df_+ncp_)*(df_+3*ncp_)/(3*squared(df_+2*ncp_));
        const Real p = (df_+2*ncp_)/squared(df_+ncp_);
        const Real m = (h-1)*(1-3*h);

        const Real u= (std::pow(x/(df_+ncp_), h) - (1 + h*p*(h-1-0.5*(2-h)*m*p)))/
            (h*std::sqrt(2*p)*(1+0.5*m*p));

        return CumulativeNormalDistribution()(u);
    }

    InverseNonCentralCumulativeChiSquareDistribution::
      InverseNonCentralCumulativeChiSquareDistribution(Real df, Real ncp, 
                                             Size maxEvaluations, 
                                             Real accuracy)
    : nonCentralDist_(df, ncp),
      guess_(df+ncp),
      maxEvaluations_(maxEvaluations),
      accuracy_(accuracy) {
    }

    Real InverseNonCentralCumulativeChiSquareDistribution::operator()(Real x) const {

        // first find the right side of the interval
        Real upper = guess_;
        Size evaluations = maxEvaluations_;
        while (nonCentralDist_(upper) < x && evaluations > 0) {
            upper*=2.0;
            --evaluations;
        }

        // use a Brent solver for the rest
        Brent solver;
        solver.setMaxEvaluations(evaluations);
        return solver.solve([&](Real y) { return nonCentralDist_(y) - x; },
                            accuracy_, 0.75*upper, 
                            (evaluations == maxEvaluations_)? 0.0: Real(0.5*upper),
                            upper);
    }
}
