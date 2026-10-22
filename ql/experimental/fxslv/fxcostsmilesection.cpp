/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Raghav Subbarao

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

#include <algorithm>
#include <complex>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/array.hpp>
#include <ql/math/matrixutilities/svd.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/experimental/fxslv/fxcostsmilesection.hpp>

namespace QuantLib {

    Integer CubicPolynomial::roots(std::vector<Real>& zeros)
    {
        Real pi = 3.14159265358979323846;
        const std::vector<Real> c = coefficients();
        if (c[3] == 0.0 || std::abs(c[3] / c[2]) < 1.0e-6)
        {
            quadratic q(c[2], c[1], c[0]);
            Real r1, r2;
            if (q.roots(r1, r2))
            {
                    zeros.clear();
                    zeros.push_back(r1);
                    zeros.push_back(r2);
                    return 2;
            }
        }

        Real B = c[2] / c[3], C = c[1] / c[3], D = c[0] / c[3];
        Real Q = (B * B - C * 3.0) / 9.0, QQQ = Q * Q * Q;
        Real R = (2.0 * B * B * B - 9.0 * B * C + 27.0 * D) / 54.0, RR = R * R;

        if (std::abs(RR - QQQ) < 1.0e-16)
        {
            // single repeated root
            zeros.clear();
            zeros.push_back(-std::pow(D, 1.0 / 3.0));
            return 3;
        }
        else if (RR <= QQQ)
        {
            // 3 real roots
            // sqrt and division is safe as RR >= 0 and QQQ > RR => QQQ > 0
            // The acos is also safe, since RR / QQQ < 1 => R / sqrt(QQQ) < 1
            Real theta = std::acos(R / std::sqrt(QQQ));

            // This is safe, since QQQ >= 0 => Q >= 0
            Real r1, r2, r3;
            r1 = r2 = r3 = -2.0 * std::sqrt(Q);
            r1 *= cos(theta / 3.0);
            r2 *= cos((theta + 2 * pi) / 3.0);
            r3 *= cos((theta - 2 * pi) / 3.0);

            r1 -= B / 3.0;
            r2 -= B / 3.0;
            r3 -= B / 3.0;

            zeros.clear();
            zeros.push_back(r1);
            zeros.push_back(r2);
            zeros.push_back(r3);
            return 3;
        }
        else
        {
            // 1 real root
            Real root;
            Real d = std::sqrt(RR - QQQ) - R;
            Real s = (d > 0) ? std::pow(d, 1.0 / 3.0) : -std::pow(-d, 1.0 / 3.0);
            if (std::fabs(s) < 1.0e-10)
            {
                d = -std::sqrt(RR - QQQ) - R;
                root = (d > 0) ? std::pow(d, 1.0 / 3.0) : -std::pow(-d, 1.0 / 3.0);
            }
            else
            {

                root = s + Q / s;
            }
            root -= B / 3.0;

            zeros.clear();
            zeros.push_back(root);
            return 1;
        }

        zeros.clear();
        return 0;
    }


    Integer QuarticPolynomial::roots(std::vector<Real>& zeros)
    {
        zeros.clear();
        const std::vector<Real> c = coefficients();

        if (c[4] == 0.0) {
            return CubicPolynomial(std::vector<Real>(c.begin() + 1, c.end())).roots(zeros);
        }

        Real B = c[3] / c[4], C = c[2] / c[4], D = c[1] / c[4], E = c[0] / c[4];
        Real BB = B * B;
        Real I = C - 0.375 * BB;
        Real J = D - B * C * 0.5 + BB * B * 0.125;
        Real K = E - B * D * 0.25 + C * BB * 0.0625 - BB * BB * 0.01171875;
        // the depressed quartic is u^4 + I * u^2 + J * u + K

        if (std::abs(J) < 1.0e-10) {
            // TODO: depressed equation is a biquadratic!
            quadratic quad(1.0, I, K);

            zeros.clear();
            if (quad.discriminant() < 0) {
                // no roots;
                return 0;
            }

            Real z1, z2;
            quad.roots(z1, z2);

            if (z1 > 0) {
                zeros.push_back(std::sqrt(z1) - 0.25 * B);
                zeros.push_back(-std::sqrt(z1) - 0.25 * B);
            }

            if (z2 > 0) {
                zeros.push_back(std::sqrt(z2) - 0.25 * B);
                zeros.push_back(-std::sqrt(z2) - 0.25 * B);
            }

            return zeros.size();
        }

        // create and solve cubic - this cubic is -J*J<0 at 0 and
        // tends to infinity as x-> infinity -> it has a positive root!
        CubicPolynomial p3(std::vector<Real>{-J * J, I * I - 4.0 * K, 2.0 * I, 1.0});

        std::vector<Real> z3;
        p3.roots(z3);

        const auto positive = std::find_if(z3.begin(), z3.end(), [](Real r) { return r >= 0.0; });
        QL_ENSURE(positive != z3.end(), "the resolvent cubic should have a non-negative root");
        Real z = *positive;

        Real p = std::sqrt(z);

        if (std::abs(p) < 1.0e-10) {
            zeros.push_back(-0.25 * B);
            return 1;

        }

        Real r = -p;
        Real q = (I + z - J / p) / 2.0;
        Real s = (I + z + J / p) / 2.0;

        quadratic quad1(1.0, p, q), quad2(1.0, r, s);

        Real z1, z2;
        if (quad1.discriminant() >= 0.0)
        {
            quad1.roots(z1, z2);
            zeros.push_back(z1 - 0.25 * B);
            zeros.push_back(z2 - 0.25 * B);
        }

        if (quad2.discriminant() >= 0.0) {
            quad2.roots(z1, z2);
            zeros.push_back(z1 - 0.25 * B);
            zeros.push_back(z2 - 0.25 * B);
        }

        return zeros.size();
    }


    // -----------------------------------------------------------------------
    //  fxCostSmileSection — constructors
    // -----------------------------------------------------------------------

    FxCostSmileSection::FxCostSmileSection(const Date& exerciseDate,
                                           const Handle<Quote>& spot,
                                           const ext::shared_ptr<FxSmileQuotes>& quotes,
                                           const Handle<YieldTermStructure>& foreignDiscount,
                                           const Handle<YieldTermStructure>& domesticDiscount,
                                           DeltaVolQuote::DeltaType deltaType,
                                           DeltaVolQuote::AtmType atmType,
                                           Real alpha,
                                           const DayCounter& dayCounter,
                                           const FxSettleConvention& settlement,
                                           const Date& referenceDate,
                                           bool weightedCalibrationFlag)
    : FxSmileSectionByStrike(exerciseDate, spot, quotes,
                             foreignDiscount, domesticDiscount,
                             deltaType, atmType, dayCounter, settlement, referenceDate),
        weightedCalibrationFlag_(weightedCalibrationFlag), alpha_(alpha)
    {
        QL_REQUIRE(alpha >= 0.0 && alpha <= 1.0, "alpha must be in [0,1], got " << alpha);
        params_.reserve(4);
    }

    FxCostSmileSection::FxCostSmileSection(Time exerciseTime,
                                           const Handle<Quote>& spot,
                                           const ext::shared_ptr<FxSmileQuotes>& quotes,
                                           const Handle<YieldTermStructure>& foreignDiscount,
                                           const Handle<YieldTermStructure>& domesticDiscount,
                                           DeltaVolQuote::DeltaType deltaType,
                                           DeltaVolQuote::AtmType atmType,
                                           Real alpha,
                                           const DayCounter& dayCounter,
                                           bool weightedCalibrationFlag)
    : FxSmileSectionByStrike(exerciseTime, spot, quotes,
                             foreignDiscount, domesticDiscount,
                             deltaType, atmType, dayCounter),
        weightedCalibrationFlag_(weightedCalibrationFlag), alpha_(alpha)
    {
        QL_REQUIRE(alpha >= 0.0 && alpha <= 1.0, "alpha must be in [0,1], got " << alpha);
        params_.reserve(4);
    }


    // -----------------------------------------------------------------------
    //  fxCostSmileSection — calibration (linear SVD, general alpha)
    // -----------------------------------------------------------------------

    Array FxCostSmileSection::initialParams() const
    {
        // Not used — calibration is algebraic (SVD).
        Array guess(4, 0.0);
        guess[0] = 1.0;
        return guess;
    }

    void FxCostSmileSection::calibrate() const {
        // Build the regressor matrix for the general-alpha cost equation.
        //
        // The cost equation at a quoted point with total vol w = sigma * sqrt(T) is:
        //
        //   Cgamma + 2*Ctheta*w^(1+alpha) - 2*Ceta*d+(w)*w^alpha + Comega*d+(w)*d-(w)*w^(2*alpha) = 0
        //
        // Columns: [Cgamma, Ctheta, Ceta, Comega]
        // Regressors: [1, 2*w^(1+a), -2*d+*w^a, d+*d-*w^(2a)]   (times optional weight)
        //
        // Note: d+ is used for the vanna regressor in both the standard and premium-adjusted
        // cases — this is consistent with the existing per-class behaviour preserved here.

        auto dp = [&](Real k, Real iw) { return std::log(fwd_ / k) / iw + iw / 2.; };
        auto dm = [&](Real k, Real iw) { return std::log(fwd_ / k) / iw - iw / 2.; };

        Real htau = std::sqrt(exerciseTime());

        std::vector<Real> ts;
        Real w, k, wt;
        CumulativeNormalDistribution f;

        for (Size i = 0; i < targets_.size(); i++) {
            // the closed form needs points on the smile (delta-vol quotes
            // or smile strangles); broker strangles are not points
            const auto point = targets_[i]->point(*this);
            QL_REQUIRE(point, "cost smile sections can only be calibrated to points on the "
                              "smile (delta-vol quotes or smile strangles), not to broker "
                              "strangles");
            k = point->first;
            w = point->second * htau;

            wt = weightedCalibration() ? f.derivative(dp(k, w)) : 1.0;

            ts.push_back(1.0 * wt);
            ts.push_back(2.0 * std::pow(w, 1.0 + alpha_) * wt);
            if (premiumAdjust()) {
                ts.push_back(-2.0 * dp(k, w) * std::pow(w, alpha_) * wt);
            } else {
                ts.push_back(-2.0 * dm(k, w) * std::pow(w, alpha_) * wt);
            }
            
            ts.push_back(dp(k, w) * dm(k, w) * std::pow(w, 2.0 * alpha_) * wt);
        }

        Matrix A(targets_.size(), 4, ts.begin(), ts.end());
        SVD svd(A);

        const Matrix V = svd.V();

        params_.clear();
        params_.push_back(1.0);
        params_.push_back(V(1, 3) / V(0, 3));
        params_.push_back(V(2, 3) / V(0, 3));
        params_.push_back(V(3, 3) / V(0, 3));
    }


    // -----------------------------------------------------------------------
    //  fxCostSmileSection — Newton-Raphson vol by strike (general alpha)
    // -----------------------------------------------------------------------

    Volatility FxCostSmileSection::volByStrikeImpl(Real strike,
                                                   Real fwd,
                                                   Time tau,
                                                   const std::vector<Real>& params) const
    {
        // Solve f(rho) = 0 for total vol rho = sigma*sqrt(tau) at log-moneyness x = ln(F/K).
        //
        // The cost equation with d(rho) = c * rho^alpha dynamics is:
        //
        //   f(rho) = p0 + (2*p1 - s*p2)*rho^(1+a) - 2*p2*x*rho^(a-1)
        //          + p3*x^2*rho^(2a-2) - (p3/4)*rho^(2+2a)  =  0
        // => p3*x^2 - 2*p2*x*rho^(1-a) + p0*rho^(2-2a) + (2*p1 - s*p2)*rho^(3-a) - (p3/4)*rho^4
        //
        // where  s = -1 (standard, d- in vanna term) or +1 (premium-adjusted, d+ in vanna term).
        // p = [Cgamma, Ctheta, Ceta, Comega] normalised so that p0 = 1.

        Real x = std::log(fwd / strike);
        Real a = alpha_;
        Real s = premiumAdjust() ? 1.0 : -1.0;

        auto f = [&](Real rho) -> Real {
            return params[3] * x * x
                 - 2.0 * params[2] * x * std::pow(rho, 1.0 - a)
                 + params[0] * std::pow(rho, 2.0 - 2.0 * a)
                 + (2.0 * params[1] - s  * params[2]) * std::pow(rho, 3.0 - a)
                 - (params[3] / 4.0) * std::pow(rho, 4.0);
        };

        auto df = [&](Real rho) -> Real {
            return 2.0 * params[2] * x * (a - 1.0) * std::pow(rho, -a)
                 + 2.0 * params[0] * (1.0 - a) * std::pow(rho, 1.0 - 2.0 * a) 
                 + (2.0 * params[1] - s * params[2]) * (3.0 - a) * std::pow(rho, 2.0 - a)
                 - params[3] * std::pow(rho, 3.0);
        };

        // Start from the reference total vol.
        Real rho = referenceVol() * std::sqrt(tau);

        static const Size maxIter = 100;
        static const Real tol = 1.0e-12;

        for (Size i = 0; i < maxIter; ++i) {
            Real fval = f(rho);
            if (std::fabs(fval) < tol)
                break;
            Real dfval = df(rho);
            QL_REQUIRE(std::fabs(dfval) > 1.0e-20,
                       "near-zero derivative in Newton-Raphson for fxCostSmileSection");
            Real step = -fval / dfval;
            // Halve the step until rho stays strictly positive.
            while (rho + step <= 0.0)
                step *= 0.5;
            rho += step;
        }

        // the loop also ends on running out of iterations; never return a
        // vol that is not on the smile
        QL_ENSURE(std::fabs(f(rho)) < tol,
                  "Newton-Raphson for fxCostSmileSection did not converge at strike "
                      << strike << " after " << maxIter << " iterations: residual " << f(rho));

        return rho / std::sqrt(tau);
    }


    // -----------------------------------------------------------------------
    //  fxCostSmileSectionFlatDynamics — constructors (forward alpha=0)
    // -----------------------------------------------------------------------

    FxCostSmileSectionFlatDynamics::FxCostSmileSectionFlatDynamics(const Date& exerciseDate,
                                                                   const Handle<Quote>& spot,
                                                                   const ext::shared_ptr<FxSmileQuotes>& quotes,
                                                                   const Handle<YieldTermStructure>& foreignDiscount,
                                                                   const Handle<YieldTermStructure>& domesticDiscount,
                                                                   DeltaVolQuote::DeltaType deltaType,
                                                                   DeltaVolQuote::AtmType atmType,
                                                                   const DayCounter& dayCounter,
                                                                   const FxSettleConvention& settlement,
                                                                   const Date& referenceDate,
                                                                   bool weightedCalibrationFlag)
    : FxCostSmileSection(exerciseDate, spot, quotes,
                         foreignDiscount, domesticDiscount,
                         deltaType, atmType, 0.0,
                         dayCounter, settlement, referenceDate, weightedCalibrationFlag) {}

    FxCostSmileSectionFlatDynamics::FxCostSmileSectionFlatDynamics(Time exerciseTime,
                                                                   const Handle<Quote>& spot,
                                                                   const ext::shared_ptr<FxSmileQuotes>& quotes,
                                                                   const Handle<YieldTermStructure>& foreignDiscount,
                                                                   const Handle<YieldTermStructure>& domesticDiscount,
                                                                   DeltaVolQuote::DeltaType deltaType,
                                                                   DeltaVolQuote::AtmType atmType,
                                                                   const DayCounter& dayCounter,
                                                                   bool weightedCalibrationFlag)
    : FxCostSmileSection(exerciseTime, spot, quotes,
                         foreignDiscount, domesticDiscount,
                         deltaType, atmType, 0.0,
                         dayCounter, weightedCalibrationFlag) {}


    // -----------------------------------------------------------------------
    //  fxCostSmileSectionFlatDynamics — quartic vol by strike (alpha = 0)
    // -----------------------------------------------------------------------

    Volatility FxCostSmileSectionFlatDynamics::volByStrikeImpl(Real strike,
                                                               Real fwd,
                                                               Time tau,
                                                               const std::vector<Real>& params) const
    {
        Real x = std::log(fwd / strike);
        Integer s = premiumAdjust() ? 1 : -1;

        QuarticPolynomial q(std::vector<Real>{-x * x * params[3], 2 * x * params[2], -params[0],
                                    s * params[2] - 2 * params[1], params[3] / 4.});
        std::vector<Real> omega;
        q.roots(omega);

        // the first non-negative real root; which root to take when there
        // are several is an open question
        const auto root = std::find_if(omega.begin(), omega.end(), [](Real r) { return r >= 0.0; });
        QL_REQUIRE(root != omega.end(),
                   "cost equation has no non-negative real root at strike " << strike);
        Real w = *root;

        // improve root if necessary
        Real err = q(w);
        if (std::abs(err) > 1.0e-16) {
            Brent solver;
            w = solver.solve([&](Real x) { return q(x); }, 1e-16, w, w - 0.001 * std::sqrt(tau), w + 0.001 * std::sqrt(tau));
        }

        return w / std::sqrt(tau);
    }


    // -----------------------------------------------------------------------
    //  fxCostSmileSectionScaledDynamics — constructors (forward alpha=1)
    // -----------------------------------------------------------------------

    FxCostSmileSectionScaledDynamics::FxCostSmileSectionScaledDynamics(const Date& exerciseDate,
                                                                       const Handle<Quote>& spot,
                                                                       const ext::shared_ptr<FxSmileQuotes>& quotes,
                                                                       const Handle<YieldTermStructure>& foreignDiscount,
                                                                       const Handle<YieldTermStructure>& domesticDiscount,
                                                                       DeltaVolQuote::DeltaType deltaType,
                                                                       DeltaVolQuote::AtmType atmType,
                                                                       const DayCounter& dayCounter,
                                                                       const FxSettleConvention& settlement,
                                                                       const Date& referenceDate,
                                                                       bool weightedCalibrationFlag)
    : FxCostSmileSection(exerciseDate, spot, quotes,
                         foreignDiscount, domesticDiscount,
                         deltaType, atmType, 1.0,
                         dayCounter, settlement, referenceDate, weightedCalibrationFlag) {}

    FxCostSmileSectionScaledDynamics::FxCostSmileSectionScaledDynamics(Time exerciseTime,
                                                                       const Handle<Quote>& spot,
                                                                       const ext::shared_ptr<FxSmileQuotes>& quotes,
                                                                       const Handle<YieldTermStructure>& foreignDiscount,
                                                                       const Handle<YieldTermStructure>& domesticDiscount,
                                                                       DeltaVolQuote::DeltaType deltaType,
                                                                       DeltaVolQuote::AtmType atmType,
                                                                       const DayCounter& dayCounter,
                                                                       bool weightedCalibrationFlag)
    : FxCostSmileSection(exerciseTime, spot, quotes,
                         foreignDiscount, domesticDiscount,
                         deltaType, atmType, 1.0,
                         dayCounter, weightedCalibrationFlag) {}


    // -----------------------------------------------------------------------
    //  fxCostSmileSectionScaledDynamics — quadratic vol by strike (alpha = 1)
    // -----------------------------------------------------------------------

    Volatility FxCostSmileSectionScaledDynamics::volByStrikeImpl(Real strike,
                                                                 Real fwd,
                                                                 Time tau,
                                                                 const std::vector<Real>& params) const
    {
        Real x = std::log(fwd / strike);
        Integer s = premiumAdjust() ? 1 : -1;

        quadratic q(params[3] / 4., s * params[2] - 2 * params[1],
                    2 * x * params[2] - params[0] - x * x * params[3]);
        Real y, z;
        bool hasroots = q.roots(y, z);

        QL_REQUIRE(hasroots, "calibrated smile should have a real root");

        Real w = y;
        if (w < 0)
            w = z;

        return std::sqrt(w / tau);
    }

    // -----------------------------------------------------------------------
    //  FxCostSmileSectionStickyDelta — constructors (forward alpha=0)
    // -----------------------------------------------------------------------

    FxCostSmileSectionStickyDelta::FxCostSmileSectionStickyDelta(const Date& exerciseDate,
                                                                 const Handle<Quote>& spot,
                                                                 const ext::shared_ptr<FxSmileQuotes>& quotes,
                                                                 const Handle<YieldTermStructure>& foreignDiscount,
                                                                 const Handle<YieldTermStructure>& domesticDiscount,
                                                                 DeltaVolQuote::DeltaType deltaType,
                                                                 DeltaVolQuote::AtmType atmType,
                                                                 const DayCounter& dayCounter,
                                                                 const FxSettleConvention& settlement,
                                                                 const Date& referenceDate,
                                                                 bool weightedCalibrationFlag)
    : FxCostSmileSection(exerciseDate, spot, quotes,
                         foreignDiscount, domesticDiscount,
                         deltaType, atmType, 0.0,
                         dayCounter, settlement, referenceDate, weightedCalibrationFlag) {}

    FxCostSmileSectionStickyDelta::FxCostSmileSectionStickyDelta(Time exerciseTime,
                                                                 const Handle<Quote>& spot,
                                                                 const ext::shared_ptr<FxSmileQuotes>& quotes,
                                                                 const Handle<YieldTermStructure>& foreignDiscount,
                                                                 const Handle<YieldTermStructure>& domesticDiscount,
                                                                 DeltaVolQuote::DeltaType deltaType,
                                                                 DeltaVolQuote::AtmType atmType,
                                                                 const DayCounter& dayCounter,
                                                                 bool weightedCalibrationFlag)
    : FxCostSmileSection(exerciseTime, spot, quotes,
                         foreignDiscount, domesticDiscount,
                         deltaType, atmType, 0.0,
                         dayCounter, weightedCalibrationFlag) {}


    void FxCostSmileSectionStickyDelta::calibrate() const {
        // Build the regressor matrix for the general-alpha cost equation.
        //
        // The cost equation at a quoted point with total vol w = sigma * sqrt(T) is:
        //
        //   Cgamma + 2*Ctheta*w^(1+alpha) - 2*Ceta*d+(w)*w^alpha + Comega*d+(w)*d-(w)*w^(2*alpha) =
        //   0
        //
        // Columns: [Cgamma, Ctheta, Ceta, Comega]
        // Regressors: [1, 2*w^(1+a), -2*d+*w^a, d+*d-*w^(2a)]   (times optional weight)
        //
        // Note: d+ is used for the vanna regressor in both the standard and premium-adjusted
        // cases — this is consistent with the existing per-class behaviour preserved here.

        auto dp = [&](Real k, Real iw) { return std::log(forward() / k) / iw + iw / 2.; };
        auto dm = [&](Real k, Real iw) { return std::log(forward() / k) / iw - iw / 2.; };

        Real htau = std::sqrt(exerciseTime());

        // check all quotes are delta-vol quotes - cannot be calibrated otherwise
        for (Size i = 0; i < targets_.size(); i++) {
            // the closed form needs points on the smile (delta-vol quotes
            // or smile strangles); broker strangles are not points
            const auto point = targets_[i]->point(*this);
            QL_REQUIRE(point, "cost smile sections can only be calibrated to points on the "
                              "smile (delta-vol quotes or smile strangles), not to broker "
                              "strangles");
        }

        // sort the targets by strike
        auto sortByStrike = [&](const ext::shared_ptr<FxSmileTarget>& L,
                                const ext::shared_ptr<FxSmileTarget>& R) {
            return L->point(*this)->first < R->point(*this)->first;
        };
        std::sort(targets_.begin(), targets_.end(), sortByStrike);

        std::vector<std::optional<std::pair<Rate, Volatility>>> points;
        for (Size i = 0; i < targets_.size(); i++) {
            points.push_back(targets_[i]->point(*this));
        }

        std::vector<Real> slopes;
        Real sd, su;
        auto discreteSlope = [&](const std::optional<std::pair<Rate, Volatility>>& p1,
                                 const std::optional<std::pair<Rate, Volatility>>& p2) {
            return (p2->second - p1->second) * htau * (p2->first + p1->first) /
                   (2 * (p2->first - p1->first));
        };

        // initialize slope estimates
        slopes.push_back(discreteSlope(points[0], points[1]));
        for (Size i = 1; i < targets_.size() - 1; i++) {
            sd = discreteSlope(points[i - 1], points[i]);
            su = discreteSlope(points[i], points[i + 1]);
            slopes.push_back((sd + su) / 2.);
        }
        // final one
        slopes.push_back(discreteSlope(points[targets_.size() - 2], points[targets_.size() - 1]));

        // Precompute all per-point quantities that are constant across iterations.
        // Only slopes[i] changes; everything else depends only on quotes and forward.
        struct PointCache {
            Real k, w, x;      // strike, total vol, log-moneyness
            Real phi_p, phi_m; // d+, d-
            Real phi_v;        // d+ (premium-adjusted) or d- (standard) for vanna regressor
            Real wt;           // calibration weight
        };

        CumulativeNormalDistribution f;
        const Real pa = premiumAdjust() ? 1. : -1.;

        // pre-compute value for iterative procedure below
        std::vector<PointCache> cache;
        cache.reserve(targets_.size());
        for (Size i = 0; i < targets_.size(); i++) {
            PointCache c;
            c.k = points[i]->first;
            c.w = points[i]->second * htau;
            c.x = std::log(forward() / c.k);
            c.phi_p = dp(c.k, c.w);
            c.phi_m = dm(c.k, c.w);
            c.phi_v = premiumAdjust() ? c.phi_p : c.phi_m;
            c.wt = weightedCalibration() ? f.derivative(c.phi_p) : 1.0;
            cache.push_back(c);
        }

        std::vector<Real> ts, ss;
        Real eps;
        do {
            eps = 0.0;

            ts.clear();
            std::vector<Real> normCost(targets_.size());
            // cost-equation rows (n rows)
            for (Size i = 0; i < targets_.size(); i++) {
                const PointCache& c = cache[i];
                const Real beta = slopes[i];
                const Real r1 = c.wt * (1 + 2 * beta * c.phi_m + beta * beta * c.phi_p * c.phi_m);
                const Real r2 = c.wt * 2.0 * c.w;
                const Real r3 = -c.wt * 2.0 * (c.phi_v + beta * c.phi_p * c.phi_m);
                const Real r4 = c.wt * c.phi_p * c.phi_m;
                ts.push_back(r1);
                ts.push_back(r2);
                ts.push_back(r3);
                ts.push_back(r4);
                //normCost[i] = std::sqrt(r1 * r1 + r2 * r2 + r3 * r3 + r4 * r4);
            }

            // slope condition rows
            for (Size i = 0; i < targets_.size(); i++) {
                const PointCache& c = cache[i];
                const Real beta = slopes[i];
                const Real w = c.w, x = c.x;
                const Real w2 = w * w, w3 = w2 * w;
                const Real s1 = c.wt * beta * beta * w2 * (3.0 + beta * w);
                const Real s2 = c.wt * (-6.0 * beta * w2);
                const Real s3 = c.wt * (beta * w2 * (3.0 * pa - 2.0 * beta * w) - 2.0 * w);
                const Real s4 = c.wt * (beta * w3 + 2.0 * x);
                //const Real normS = std::sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4) + 1e-14;
                const Real scale = 1.0; // normCost[i] / normS;
                ts.push_back(scale * s1);
                ts.push_back(scale * s2);
                ts.push_back(scale * s3);
                ts.push_back(scale * s4);
            }

            Matrix A(2 * targets_.size(), 4, ts.begin(), ts.end());
            SVD svd(A);
            const Matrix V = svd.V();
            const Array s = svd.singularValues();

            eps += (params_[1] - V(1, 3) / V(0, 3)) * (params_[1] - V(1, 3) / V(0, 3));
            eps += (params_[2] - V(2, 3) / V(0, 3)) * (params_[2] - V(2, 3) / V(0, 3));
            eps += (params_[3] - V(3, 3) / V(0, 3)) * (params_[3] - V(3, 3) / V(0, 3));
            //eps = s[3];

            params_.clear();
            params_.push_back(1.0);
            params_.push_back(V(1, 3) / V(0, 3));
            params_.push_back(V(2, 3) / V(0, 3));
            params_.push_back(V(3, 3) / V(0, 3));

            for (Size i = 0; i < targets_.size(); i++) {
                const PointCache& c = cache[i];
                
                // form the cubic in slopes and solve
                CubicPolynomial P(std::vector<Real>{
                    2 * (c.x * params_[3] - c.w * params_[2]),
                    c.w * c.w * (c.w * params_[3] + 3 * (pa * params_[2]  - 2 * params_[1])) -
                        2 * c.x * params_[2],
                    c.w * c.w * (3 * params_[0]  - 2 * params_[2] * c.w), 
                    c.w * c.w * c.w * params_[0]});

                P.roots(ss);

                // the the closest root to the previous guess
                Real target = slopes[i];
                auto closest = std::min_element(ss.begin(), ss.end(), [target](double a, double b) {
                    return std::abs(a - target) < std::abs(b - target);
                });
                eps += (slopes[i] - *closest) * (slopes[i] - *closest);
                slopes[i] = *closest;
            }
            
        } while (eps > 1.0e-12);

        // set-up a quadratic for initializing slope
        ts.clear();
        for (Size i = 0; i < targets_.size(); i++) {
            const PointCache& c = cache[i];
            ts.push_back(c.x);
            ts.push_back(1.);
        }

        Matrix A(targets_.size(), 2, ts.begin(), ts.end());
        Matrix b(targets_.size(), 1, slopes.begin(), slopes.end());
        Matrix v = inverse(transpose(A) * A) * transpose(A) * b;

        params_.push_back(v[0][0]);
        params_.push_back(v[1][0]);

    }

    Volatility FxCostSmileSectionStickyDelta::volByStrikeImpl(
        Real strike, Real fwd, Time tau, const std::vector<Real>& params) const {
        Real x = std::log(fwd / strike);
        Integer pa = premiumAdjust() ? 1 : -1;

        Real beta = params[4] * x + params[5];
        Real eps = 0.0;
        do {
            const Real q = params[0] * beta * beta - 2 * params[2] * beta + params[3];

            QuarticPolynomial Q(std::vector<Real>{x * x * q, 2 * x * (beta * params[0] - params[2]), params[0],
                                  2 * params[1] - pa * params[2] - beta * params[0], -q / 4.});
            std::vector<Real> omega;
            Q.roots(omega);

            const auto root =
                std::find_if(omega.begin(), omega.end(), [](Real r) { return r >= 0.0; });
            QL_REQUIRE(root != omega.end(),
                       "cost equation has no non-negative real root at strike " << strike);
            const Real w = *root;

            // now form the cubic in slopes and solve to update slope
            CubicPolynomial P(std::vector<Real>{
                2 * (x * params[3] - w * params[2]),
                w * w * (w * params[3] + 3 * (pa * params[2]  - 2 * params[1])) - 2 * x * params[2],
                w * w * (3 * params[0] - 2 * params[2] * w), w * w * w * params[0]});

            std::vector<Real> ss;
            P.roots(ss);

            // the the closest root to the previous guess
            auto closest = std::min_element(ss.begin(), ss.end(), [beta](double a, double b) {
                return std::abs(a - beta) < std::abs(b - beta);
            });
            const Real prevBeta = beta;
            beta = *closest;
            eps = (beta - prevBeta) * (beta - prevBeta);
        } while (eps > 1.0e-12);

        // run one final root finding with the converged slope
        Real q = params[0] * beta * beta - 2 * params[2] * beta + params[3];

        QuarticPolynomial Q(std::vector<Real>{x * x * q, 2 * x * (beta * params[0] - params[2]),
                                              params[0], 2 * params[1] - pa * params[2] - beta * params[0], -q / 4.});
        std::vector<Real> omega;
        Q.roots(omega);

        const auto root = std::find_if(omega.begin(), omega.end(), [](Real r) { return r >= 0.0; });
        QL_REQUIRE(root != omega.end(),
                   "cost equation has no non-negative real root at strike " << strike);
        const Real w = *root;

        return w / std::sqrt(tau);
    }
}
