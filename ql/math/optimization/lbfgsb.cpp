/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Colin Alberts

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

#include <ql/math/matrix.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/lbfgsb.hpp>
#include <ql/math/optimization/problem.hpp>
#include <algorithm>
#include <cmath>
#include <deque>
#include <utility>
#include <vector>

namespace QuantLib {

    namespace {

        const Real INF = QL_MAX_REAL;

        // A bound is "absent" when it equals +/- DBL_MAX (the value
        // returned by the default Constraint and by NoConstraint).
        inline bool noUpper(Real u) {
            return u >= 0.5 * INF;
        }
        inline bool noLower(Real l) {
            return l <= -0.5 * INF;
        }

        // Compact limited-memory representation of the BFGS Hessian
        // approximation  B = theta I - W M W^T   (Byrd, Lu, Nocedal & Zhu
        // 1995, eq. 3.5), built from the stored correction pairs.
        struct CompactRep {
            Matrix W; // n x 2col,   [ Y | theta S ]
            Matrix M; // 2col x 2col, inverse of the middle matrix
            Real theta = 1.0;
            Size col = 0;
        };

        CompactRep
        buildCompactRep(const std::deque<Array>& S, const std::deque<Array>& Y, Real theta) {
            CompactRep rep;
            rep.theta = theta;
            rep.col = S.size();
            Size col = rep.col;
            if (col == 0)
                return rep;

            Size n = S.front().size();
            Matrix W(n, 2 * col);
            for (Size j = 0; j < col; ++j)
                for (Size i = 0; i < n; ++i) {
                    W[i][j] = Y[j][i];
                    W[i][col + j] = theta * S[j][i];
                }

            // small Gram matrices S^T S and S^T Y
            Matrix StS(col, col), StY(col, col);
            for (Size i = 0; i < col; ++i)
                for (Size j = 0; j < col; ++j) {
                    StS[i][j] = DotProduct(S[i], S[j]);
                    StY[i][j] = DotProduct(S[i], Y[j]);
                }

            // middle matrix  [ -D  L^T ; L  theta S^T S ]  with
            // D = diag(s_i.y_i) and L the strictly lower part of S^T Y.
            Matrix mid(2 * col, 2 * col, 0.0);
            for (Size i = 0; i < col; ++i) {
                mid[i][i] = -StY[i][i]; // -D
                for (Size j = 0; j < col; ++j) {
                    if (i > j)
                        mid[col + i][j] = StY[i][j]; // L
                    if (j > i)
                        mid[i][col + j] = StY[j][i]; // L^T
                    mid[col + i][col + j] = theta * StS[i][j];
                }
            }

            rep.W = W;
            rep.M = inverse(mid);
            return rep;
        }

        inline Array wRow(const Matrix& W, Size i) {
            Array r(W.columns());
            for (Size k = 0; k < r.size(); ++k)
                r[k] = W[i][k];
            return r;
        }

        // Generalized Cauchy point (Byrd et al. 1995, Algorithm CP).
        // Walks the breakpoints of the projected steepest-descent path
        // x(t) = P(x - t g, l, u) and locates the first local minimizer of
        // the quadratic model along it.  Returns the Cauchy point xcp, the
        // accumulated c = W^T (xcp - x) needed by the subspace step, and
        // the set of variables left free (not pinned at a bound).
        void generalizedCauchyPoint(const Array& x,
                                    const Array& g,
                                    const Array& lo,
                                    const Array& hi,
                                    const CompactRep& rep,
                                    Array& xcp,
                                    Array& cOut,
                                    std::vector<bool>& isFree) {
            Size n = x.size();
            Size m2 = 2 * rep.col;

            xcp = x;
            isFree.assign(n, true);

            Array t(n), d(n, 0.0);
            std::vector<Size> brk; // indices with a strictly positive breakpoint
            for (Size i = 0; i < n; ++i) {
                Real ti;
                if (g[i] < 0.0)
                    ti = noUpper(hi[i]) ? INF : (x[i] - hi[i]) / g[i];
                else if (g[i] > 0.0)
                    ti = noLower(lo[i]) ? INF : (x[i] - lo[i]) / g[i];
                else
                    ti = INF;
                t[i] = ti;
                if (ti <= 0.0) {
                    // already sitting on the bound the gradient pushes into
                    d[i] = 0.0;
                    isFree[i] = false;
                } else {
                    d[i] = -g[i];
                    brk.push_back(i);
                }
            }

            Array c(m2, 0.0);
            if (brk.empty()) { // every variable is pinned
                cOut = c;
                return;
            }

            std::sort(brk.begin(), brk.end(), [&t](Size a, Size b) { return t[a] < t[b]; });

            Array p = (rep.col > 0) ? (d * rep.W) : Array(0);

            Real fp = -DotProduct(d, d); // first derivative of the model, m'(0)
            Real fpp = -rep.theta * fp;  // second derivative, theta d^T d - ...
            if (rep.col > 0)
                fpp -= DotProduct(p, rep.M * p);
            Real fppFloor = QL_EPSILON * (fpp > 0.0 ? fpp : 1.0);
            Real dtMin = (fpp > 0.0) ? -fp / fpp : 0.0;

            Real tOld = 0.0;
            Size ptr = 0;
            Size b = brk[ptr];
            Real tb = t[b];
            Real dt = tb;
            bool exhausted = false;

            while (dtMin >= dt && tb < INF) {
                // pin variable b at the bound it has reached
                Real xcpb = (d[b] > 0.0) ? hi[b] : (d[b] < 0.0 ? lo[b] : x[b]);
                xcp[b] = xcpb;
                isFree[b] = false;
                Real zb = xcpb - x[b];
                Real gb = g[b];

                if (rep.col > 0) {
                    c += dt * p;
                    Array wb = wRow(rep.W, b);
                    Array Mc = rep.M * c;
                    Array Mp = rep.M * p;
                    Array Mwb = rep.M * wb;
                    fp += dt * fpp + gb * gb + rep.theta * gb * zb - gb * DotProduct(wb, Mc);
                    fpp += -rep.theta * gb * gb - 2.0 * gb * DotProduct(wb, Mp) -
                           gb * gb * DotProduct(wb, Mwb);
                    p += gb * wb;
                } else {
                    fp += dt * fpp + gb * gb + rep.theta * gb * zb;
                    fpp += -rep.theta * gb * gb;
                }
                if (fpp < fppFloor)
                    fpp = fppFloor;
                d[b] = 0.0;
                dtMin = (fpp > 0.0) ? -fp / fpp : 0.0;
                tOld = tb;

                if (++ptr >= brk.size()) {
                    exhausted = true;
                    break;
                }
                b = brk[ptr];
                tb = t[b];
                dt = tb - tOld;
            }

            // advance the still-free variables along the final segment
            if (exhausted)
                dtMin = 0.0;
            dtMin = std::max(dtMin, Real(0.0));
            tOld += dtMin;
            for (Size i = 0; i < n; ++i)
                if (isFree[i])
                    xcp[i] = x[i] + tOld * d[i];
            if (rep.col > 0)
                c += dtMin * p;
            cOut = c;
        }

        // Subspace minimization by the direct primal method (Byrd et al.
        // 1995, section 5.1).  Minimizes the quadratic model over the free
        // variables, holding the active ones at their Cauchy-point bound,
        // then truncates the step back into the box.
        Array subspaceMinimization(const Array& x,
                                   const Array& g,
                                   const Array& xcp,
                                   const Array& c,
                                   const Array& lo,
                                   const Array& hi,
                                   const CompactRep& rep,
                                   const std::vector<bool>& isFree) {
            Size n = x.size();
            std::vector<Size> freeIdx;
            for (Size i = 0; i < n; ++i)
                if (isFree[i])
                    freeIdx.push_back(i);
            Size nf = freeIdx.size();

            Array xbar = xcp;
            if (nf == 0)
                return xbar;

            Real theta = rep.theta;
            Size m2 = 2 * rep.col;

            // reduced gradient of the model at the Cauchy point:
            //   r = [ g + theta (xcp - x) - W M c ]  restricted to free vars
            Array WMc(n, 0.0);
            if (rep.col > 0)
                WMc = rep.W * (rep.M * c);

            Array r(nf);
            for (Size k = 0; k < nf; ++k) {
                Size i = freeIdx[k];
                r[k] = g[i] + theta * (xcp[i] - x[i]) - WMc[i];
            }

            Array dhat(nf);
            if (rep.col == 0) {
                for (Size k = 0; k < nf; ++k)
                    dhat[k] = -r[k] / theta;
            } else {
                // v = M (W_free^T r)
                Array wtr(m2, 0.0);
                for (Size k = 0; k < nf; ++k) {
                    Size i = freeIdx[k];
                    for (Size j = 0; j < m2; ++j)
                        wtr[j] += rep.W[i][j] * r[k];
                }
                Array v = rep.M * wtr;

                // N = I - (1/theta) M (W_free^T W_free)
                Matrix WftWf(m2, m2, 0.0);
                for (Size k = 0; k < nf; ++k) {
                    Size i = freeIdx[k];
                    for (Size a = 0; a < m2; ++a)
                        for (Size bb = 0; bb < m2; ++bb)
                            WftWf[a][bb] += rep.W[i][a] * rep.W[i][bb];
                }
                Matrix N = rep.M * WftWf;
                N *= -1.0 / theta;
                for (Size a = 0; a < m2; ++a)
                    N[a][a] += 1.0;
                v = inverse(N) * v;

                // dhat = -(1/theta) r - (1/theta^2) W_free v
                for (Size k = 0; k < nf; ++k) {
                    Size i = freeIdx[k];
                    Real Wfv = 0.0;
                    for (Size j = 0; j < m2; ++j)
                        Wfv += rep.W[i][j] * v[j];
                    dhat[k] = -r[k] / theta - Wfv / (theta * theta);
                }
            }

            // truncate so that every free variable stays inside the box
            Real alphaStar = 1.0;
            for (Size k = 0; k < nf; ++k) {
                Size i = freeIdx[k];
                if (dhat[k] > 0.0 && !noUpper(hi[i]))
                    alphaStar = std::min(alphaStar, (hi[i] - xcp[i]) / dhat[k]);
                else if (dhat[k] < 0.0 && !noLower(lo[i]))
                    alphaStar = std::min(alphaStar, (lo[i] - xcp[i]) / dhat[k]);
            }
            alphaStar = std::max(alphaStar, Real(0.0));

            for (Size k = 0; k < nf; ++k)
                xbar[freeIdx[k]] = xcp[freeIdx[k]] + alphaStar * dhat[k];
            return xbar;
        }

        // Largest feasible step length along d starting from x.
        Real maxFeasibleStep(const Array& x, const Array& d, const Array& lo, const Array& hi) {
            Real stp = INF;
            for (Size i = 0; i < x.size(); ++i) {
                if (d[i] > 0.0 && !noUpper(hi[i]))
                    stp = std::min(stp, (hi[i] - x[i]) / d[i]);
                else if (d[i] < 0.0 && !noLower(lo[i]))
                    stp = std::min(stp, (lo[i] - x[i]) / d[i]);
            }
            return stp;
        }

        // Line search enforcing the strong Wolfe conditions (Nocedal &
        // Wright, Algorithms 3.5/3.6), capped at the largest feasible step.
        // On success returns the step, trial point, value and gradient.
        bool lineSearchWolfe(Problem& P,
                             const Array& x,
                             const Array& d,
                             Real f0,
                             const Array& g0,
                             Real stpMax,
                             Real& alpha,
                             Array& xt,
                             Real& ft,
                             Array& gt) {
            const Real c1 = 1e-4, c2 = 0.9;
            const Size maxIter = 30;

            Real dphi0 = DotProduct(g0, d);
            if (dphi0 >= 0.0)
                return false; // not a descent direction

            bool haveBest = false;
            Real bestAlpha = 0.0, bestF = f0;
            Array bestX, bestG;

            // evaluate phi(a) and phi'(a), tracking the best decrease seen
            auto eval = [&](Real a) -> Real {
                xt = x + a * d;
                ft = P.valueAndGradient(gt, xt);
                if (ft < bestF) {
                    haveBest = true;
                    bestF = ft;
                    bestAlpha = a;
                    bestX = xt;
                    bestG = gt;
                }
                return DotProduct(gt, d);
            };

            Real aLo = 0.0, aHi = 0.0, fLo = f0;
            bool bracketed = false;

            Real aPrev = 0.0, fPrev = f0;
            Real a = std::min(Real(1.0), stpMax);

            for (Size i = 0; i < maxIter; ++i) {
                Real dphi = eval(a);
                if (ft > f0 + c1 * a * dphi0 || (i > 0 && ft >= fPrev)) {
                    aLo = aPrev;
                    fLo = fPrev;
                    aHi = a;
                    bracketed = true;
                    break;
                }
                if (std::fabs(dphi) <= -c2 * dphi0) {
                    alpha = a;
                    return true; // strong Wolfe satisfied
                }
                if (dphi >= 0.0) {
                    aLo = a;
                    fLo = ft;
                    aHi = aPrev;
                    bracketed = true;
                    break;
                }
                aPrev = a;
                fPrev = ft;
                if (a >= stpMax)
                    break; // cannot expand further
                a = std::min(2.0 * a, stpMax);
            }

            if (bracketed) {
                for (Size j = 0; j < maxIter; ++j) {
                    a = 0.5 * (aLo + aHi); // bisection
                    Real dphi = eval(a);
                    if (ft > f0 + c1 * a * dphi0 || ft >= fLo) {
                        aHi = a;
                    } else {
                        if (std::fabs(dphi) <= -c2 * dphi0) {
                            alpha = a;
                            return true;
                        }
                        if (dphi * (aHi - aLo) >= 0.0)
                            aHi = aLo;
                        aLo = a;
                        fLo = ft;
                    }
                    if (std::fabs(aHi - aLo) < QL_EPSILON * std::max(Real(1.0), std::fabs(a)))
                        break;
                }
            }

            // strong Wolfe not reached: accept the best sufficient decrease
            if (haveBest) {
                alpha = bestAlpha;
                xt = bestX;
                ft = bestF;
                gt = bestG;
                return true;
            }
            return false;
        }

    }

    LBFGSB::LBFGSB(Size memory, Real pgTol, Real factr) : m_(memory), pgTol_(pgTol), factr_(factr) {
        QL_REQUIRE(memory > 0, "memory must be positive");
    }

    LBFGSB::LBFGSB(Array lowerBound, Array upperBound, Size memory, Real pgTol, Real factr)
    : m_(memory), pgTol_(pgTol), factr_(factr), lowerBound_(std::move(lowerBound)),
      upperBound_(std::move(upperBound)) {
        QL_REQUIRE(memory > 0, "memory must be positive");
        QL_REQUIRE(lowerBound_.size() == upperBound_.size(),
                   "lower and upper bound sizes are inconsistent");
    }

    EndCriteria::Type LBFGSB::minimize(Problem& P, const EndCriteria& endCriteria) {
        EndCriteria::Type ecType = EndCriteria::None;
        P.reset();

        Array x = P.currentValue();
        Size n = x.size();

        Array lo = lowerBound_.empty() ? P.constraint().lowerBound(x) : lowerBound_;
        Array hi = upperBound_.empty() ? P.constraint().upperBound(x) : upperBound_;
        QL_REQUIRE(lo.size() == n && hi.size() == n,
                   "bounds size does not match the number of variables");

        // start from a feasible point
        for (Size i = 0; i < n; ++i)
            x[i] = std::min(std::max(x[i], lo[i]), hi[i]);

        Array g(n);
        Real f = P.valueAndGradient(g, x);
        P.setCurrentValue(x);
        P.setFunctionValue(f);

        std::deque<Array> S, Y;
        Real theta = 1.0;
        Size iter = 0;

        while (true) {
            // infinity norm of the projected gradient
            Real pgInf = 0.0;
            for (Size i = 0; i < n; ++i) {
                Real proj = std::min(std::max(x[i] - g[i], lo[i]), hi[i]) - x[i];
                pgInf = std::max(pgInf, std::fabs(proj));
            }
            P.setGradientNormValue(pgInf * pgInf);

            if (pgInf < pgTol_) {
                ecType = EndCriteria::ZeroGradientNorm;
                break;
            }
            if (endCriteria.checkZeroGradientNorm(pgInf, ecType))
                break;
            if (endCriteria.checkMaxIterations(iter, ecType))
                break;

            // compact representation; drop the oldest pairs if the middle
            // matrix turns out numerically singular
            CompactRep rep;
            while (true) {
                try {
                    rep = buildCompactRep(S, Y, theta);
                    break;
                } catch (...) {
                    if (S.empty()) {
                        rep = CompactRep();
                        rep.theta = theta;
                        break;
                    }
                    S.pop_front();
                    Y.pop_front();
                }
            }

            Array xcp, c;
            std::vector<bool> isFree;
            generalizedCauchyPoint(x, g, lo, hi, rep, xcp, c, isFree);

            Array xbar;
            try {
                xbar = subspaceMinimization(x, g, xcp, c, lo, hi, rep, isFree);
            } catch (...) {
                xbar = xcp; // fall back to the Cauchy point
            }

            Array d = xbar - x;
            Real dphi0 = DotProduct(g, d);

            // fall back to the (always-descent) Cauchy direction, then to
            // the projected gradient, if the subspace step is not downhill
            if (Norm2(d) < QL_EPSILON || dphi0 >= 0.0) {
                d = xcp - x;
                dphi0 = DotProduct(g, d);
            }
            if (Norm2(d) < QL_EPSILON) {
                ecType = EndCriteria::StationaryPoint;
                break;
            }
            if (dphi0 >= 0.0) {
                for (Size i = 0; i < n; ++i)
                    d[i] = std::min(std::max(x[i] - g[i], lo[i]), hi[i]) - x[i];
                dphi0 = DotProduct(g, d);
                if (dphi0 >= 0.0 || Norm2(d) < QL_EPSILON) {
                    ecType = EndCriteria::StationaryPoint;
                    break;
                }
            }

            Real stpMax = maxFeasibleStep(x, d, lo, hi);
            if (stpMax < QL_EPSILON) {
                ecType = EndCriteria::StationaryPoint;
                break;
            }

            Real alpha;
            Array xt(n), gt(n);
            Real ft;
            if (!lineSearchWolfe(P, x, d, f, g, stpMax, alpha, xt, ft, gt)) {
                ecType = EndCriteria::StationaryFunctionValue;
                break;
            }

            // limited-memory update with the curvature safeguard
            Array s = xt - x;
            Array y = gt - g;
            Real sy = DotProduct(s, y);
            Real yy = DotProduct(y, y);
            if (yy > 0.0 && sy > QL_EPSILON * yy) {
                S.push_back(s);
                Y.push_back(y);
                if (S.size() > m_) {
                    S.pop_front();
                    Y.pop_front();
                }
                theta = yy / sy;
            }

            Real fOld = f;
            x = xt;
            f = ft;
            g = gt;
            ++iter;

            P.setCurrentValue(x);
            P.setFunctionValue(f);

            // relative function-reduction stop (SciPy's factr criterion)
            Real denom = std::max(std::max(std::fabs(fOld), std::fabs(f)), Real(1.0));
            if ((fOld - f) <= factr_ * QL_EPSILON * denom) {
                ecType = EndCriteria::StationaryFunctionValue;
                break;
            }
        }

        P.setCurrentValue(x);
        P.setFunctionValue(f);
        P.setGradientNormValue(DotProduct(g, g));
        return ecType;
    }

}
