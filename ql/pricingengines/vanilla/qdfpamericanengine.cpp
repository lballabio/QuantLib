/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Klaus Spanderen

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

/*! \file qrfpamericanengine.cpp
*/

#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/integrals/tanhsinhintegral.hpp>
#include <ql/math/interpolations/chebyshevinterpolation.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/vanilla/qdfpamericanengine.hpp>
#include <utility>
#ifndef QL_BOOST_HAS_TANH_SINH
#    include <ql/math/integrals/gausslobattointegral.hpp>
#endif

namespace QuantLib {

    QdFpLegendreScheme::QdFpLegendreScheme(
        Size l, Size m, Size n, Size p):
        m_(m), n_(n),
        fpIntegrator_(ext::make_shared<GaussLegendreIntegrator>(l)),
        exerciseBoundaryIntegrator_(
            ext::make_shared<GaussLegendreIntegrator>(p)) {

        QL_REQUIRE(m_ > 0, "at least one fixed point iteration step is needed");
        QL_REQUIRE(n_ > 0, "at least one interpolation point is needed");
    }

    Size QdFpLegendreScheme::getNumberOfChebyshevInterpolationNodes()
        const {
        return n_;
    }
    Size QdFpLegendreScheme::getNumberOfNaiveFixedPointSteps() const {
        return m_-1;
    }
    Size QdFpLegendreScheme::getNumberOfJacobiNewtonFixedPointSteps()
        const {
        return Size(1);
    }

    ext::shared_ptr<Integrator>
        QdFpLegendreScheme::getFixedPointIntegrator() const {
        return fpIntegrator_;
    }
    ext::shared_ptr<Integrator>
        QdFpLegendreScheme::getExerciseBoundaryToPriceIntegrator()
        const {
        return exerciseBoundaryIntegrator_;
    }

    QdFpTanhSinhIterationScheme::QdFpTanhSinhIterationScheme(
        Size m, Size n, Real eps)
    : m_(m), n_(n),
#ifdef QL_BOOST_HAS_TANH_SINH
      integrator_(ext::make_shared<TanhSinhIntegral>(eps))
#else
      integrator_(ext::make_shared<GaussLobattoIntegral>(
          100000, QL_MAX_REAL, 0.1*eps))
#endif
    {}

    Size QdFpTanhSinhIterationScheme::getNumberOfChebyshevInterpolationNodes()
        const {
        return n_;
    }
    Size QdFpTanhSinhIterationScheme::getNumberOfNaiveFixedPointSteps() const {
        return m_-1;
    }
    Size QdFpTanhSinhIterationScheme::getNumberOfJacobiNewtonFixedPointSteps()
        const {
        return Size(1);
    }

    ext::shared_ptr<Integrator>
    QdFpTanhSinhIterationScheme::getFixedPointIntegrator() const {
        return integrator_;
    }
    ext::shared_ptr<Integrator>
    QdFpTanhSinhIterationScheme::getExerciseBoundaryToPriceIntegrator()
        const {
        return integrator_;
    }

    QdFpLegendreTanhSinhScheme::QdFpLegendreTanhSinhScheme(
        Size l, Size m, Size n, Real eps)
    : QdFpLegendreScheme(l, m, n, 1),
      eps_(eps) {}

    ext::shared_ptr<Integrator>
    QdFpLegendreTanhSinhScheme::getExerciseBoundaryToPriceIntegrator() const {
#ifdef QL_BOOST_HAS_TANH_SINH
            return ext::make_shared<TanhSinhIntegral>(eps_);
#else
            return ext::make_shared<GaussLobattoIntegral>(
                100000, QL_MAX_REAL, 0.1*eps_);
#endif
    }


    namespace {
        using detail::QdAddOnSetup;
        using detail::TauHatSensitivities;
        using detail::computeTauHat;
        using detail::computeTauHatSensitivities;
    }

    class DqFpEquation {
      public:
        DqFpEquation(Rate _r,
                     Rate _q,
                     Volatility _vol,
                     std::function<Real(Real)> B,
                     ext::shared_ptr<Integrator> _integrator)
        : r(_r), q(_q), vol(_vol), B(std::move(B)), integrator(std::move(_integrator)) {
            const auto legendreIntegrator =
                ext::dynamic_pointer_cast<GaussLegendreIntegrator>(integrator);

            if (legendreIntegrator != nullptr) {
                x_i = legendreIntegrator->getIntegration()->x();
                w_i = legendreIntegrator->getIntegration()->weights();
            }
        }

        virtual std::pair<Real, Real> NDd(Real tau, Real b) const = 0;
        virtual std::tuple<Real, Real, Real> f(Real tau, Real b) const = 0;

        virtual ~DqFpEquation() = default;

      protected:
        std::pair<Real, Real> d(Time t, Real z) const {
            const Real v = vol * std::sqrt(t);
            const Real m = (std::log(z) + (r-q)*t)/v + 0.5*v;

            return std::make_pair(m, m-v);
        }

        Array x_i, w_i;
        const Rate r, q;
        const Volatility vol;

        const std::function<Real(Real)> B;
        const ext::shared_ptr<Integrator> integrator;

        const NormalDistribution phi;
        const CumulativeNormalDistribution Phi;
    };

    class DqFpEquation_B: public DqFpEquation {
      public:
        DqFpEquation_B(Real K,
                       Rate _r,
                       Rate _q,
                       Volatility _vol,
                       std::function<Real(Real)> B,
                       ext::shared_ptr<Integrator> _integrator);

        std::pair<Real, Real> NDd(Real tau, Real b) const override;
        std::tuple<Real, Real, Real> f(Real tau, Real b) const override;

      private:
          const Real K;
    };

    class DqFpEquation_A: public DqFpEquation {
      public:
        DqFpEquation_A(Real K,
                       Rate _r,
                       Rate _q,
                       Volatility _vol,
                       std::function<Real(Real)> B,
                       ext::shared_ptr<Integrator> _integrator);

        std::pair<Real, Real> NDd(Real tau, Real b) const override;
        std::tuple<Real, Real, Real> f(Real tau, Real b) const override;

      private:
          const Real K;
    };

    DqFpEquation_A::DqFpEquation_A(Real K,
                                   Rate _r,
                                   Rate _q,
                                   Volatility _vol,
                                   std::function<Real(Real)> B,
                                   ext::shared_ptr<Integrator> _integrator)
    : DqFpEquation(_r, _q, _vol, std::move(B), std::move(_integrator)), K(K) {}

    std::tuple<Real, Real, Real> DqFpEquation_A::f(Real tau, Real b) const {
        const Real v = vol * std::sqrt(tau);

        Real N, D;
        if (tau < squared(QL_EPSILON)) {
            if (close_enough(b, K)) {
                N = 1/(M_SQRT2*M_SQRTPI * v);
                D = N + 0.5;
            }
            else {
                N = 0.0;
                D = (b > K)? 1.0: 0.0;
            }
        }
        else {
            const Real stv = std::sqrt(tau)/vol;

            Real K12, K3;
            if (!x_i.empty()) {
                K12 = K3 = 0.0;

                for (Integer i = x_i.size()-1; i >= 0; --i) {
                    const Real y = x_i[i];
                    const Real m = 0.25*tau*squared(1+y);
                    const std::pair<Real, Real> dpm = d(m, b/B(tau-m));

                    K12 += w_i[i] * std::exp(q*tau - q*m)
                        *(0.5*tau*(y+1)*Phi(dpm.first) + stv*phi(dpm.first));
                    K3 += w_i[i] * stv*std::exp(r*tau-r*m)*phi(dpm.second);
                }
            } else {
                K12 = (*integrator)([&, this](Real y) -> Real {
                    const Real m = 0.25*tau*squared(1+y);
                    const Real df = std::exp(q*tau - q*m);

                    if (y <= 5*QL_EPSILON - 1) {
                        if (close_enough(b, B(tau-m)))
                            return df*stv/(M_SQRT2*M_SQRTPI);
                        else
                            return 0.0;
                    }
                    else {
                        const Real dp = d(m, b/B(tau-m)).first;
                        return df*(0.5*tau*(y+1)*Phi(dp) + stv*phi(dp));
                    }
                }, -1, 1);

                K3 = (*integrator)([&, this](Real y) -> Real {
                    const Real m = 0.25*tau*squared(1+y);
                    const Real df = std::exp(r*tau-r*m);

                    if (y <= 5*QL_EPSILON - 1) {
                        if (close_enough(b, B(tau-m)))
                            return df*stv/(M_SQRT2*M_SQRTPI);
                        else
                            return 0.0;
                    }
                    else
                        return df*stv*phi(d(m, b/B(tau-m)).second);
                }, -1, 1);
            }
            const std::pair<Real, Real> dpm = d(tau, b/K);
            N = phi(dpm.second)/v + r*K3;
            D = phi(dpm.first)/v + Phi(dpm.first) + q*K12;
        }

        const Real alpha = K*std::exp(-(r-q)*tau);
        Real fv;
        if (tau < squared(QL_EPSILON)) {
            if (close_enough(b, K))
                fv = alpha;
            else if (b > K)
                fv = 0.0;
            else {
                if (close_enough(q, Real(0)))
                    fv = alpha*r*((q < 0)? -1.0 : 1.0)/QL_EPSILON;
                else
                    fv = alpha*r/q;
            }
        }
        else
            fv = alpha*N/D;

        return std::make_tuple(N, D, fv);
    }

    std::pair<Real, Real> DqFpEquation_A::NDd(Real tau, Real b) const {
        Real Dd, Nd;

        if (tau < squared(QL_EPSILON)) {
            if (close_enough(b, K)) {
                const Real sqTau = std::sqrt(tau);
                const Real vol2 = vol*vol;
                Dd = M_1_SQRTPI*M_SQRT_2*(
                    -(0.5*vol2 + r-q) / (b*vol*vol2*sqTau) + 1 / (b*vol*sqTau));
                Nd = M_1_SQRTPI*M_SQRT_2 * (-0.5*vol2 + r-q)  / (b*vol*vol2*sqTau);
            }
            else
                Dd = Nd = 0.0;
        }
        else {
            const std::pair<Real, Real> dpm = d(tau, b/K);

            Dd = -phi(dpm.first) * dpm.first / (b*vol*vol*tau) +
                    phi(dpm.first) / (b*vol * std::sqrt(tau));
            Nd = -phi(dpm.second) * dpm.second / (b*vol*vol*tau);
        }

        return std::make_pair(Nd, Dd);
    }


    DqFpEquation_B::DqFpEquation_B(Real K,
                                   Rate _r,
                                   Rate _q,
                                   Volatility _vol,
                                   std::function<Real(Real)> B,
                                   ext::shared_ptr<Integrator> _integrator)
    : DqFpEquation(_r, _q, _vol, std::move(B), std::move(_integrator)), K(K) {}


    std::tuple<Real, Real, Real> DqFpEquation_B::f(Real tau, Real b) const {
        Real N, D;
        if (tau < squared(QL_EPSILON)) {
            if (close_enough(b, K))
                N = D = 0.5;
            else if (b < K)
                N = D = 0.0;
            else
                N = D = 1.0;
        }
        else {
            Real ni, di;
            if (!x_i.empty()) {
                const Real c = 0.5*tau;

                ni = di = 0.0;
                for (Integer i = x_i.size()-1; i >= 0; --i) {
                    const Real u = c*x_i[i] + c;
                    const std::pair<Real, Real> dpm = d(tau - u, b/B(u));
                    ni += w_i[i] * std::exp(r*u)*Phi(dpm.second);
                    di += w_i[i] * std::exp(q*u)*Phi(dpm.first);
                }
                ni *= c;
                di *= c;
            } else {
                ni = (*integrator)([&, this](Real u) -> Real {
                	const Real df = std::exp(r*u);
                    if (u >= tau*(1 - 5*QL_EPSILON)) {
                        if (close_enough(b, B(u)))
                            return 0.5*df;
                        else
                            return df*((b < B(u)? 0.0: 1.0));
                    }
                    else
                        return df*Phi(d(tau - u, b/B(u)).second);
                }, 0, tau);
                di = (*integrator)([&, this](Real u) -> Real {
                	const Real df = std::exp(q*u);
                    if (u >= tau*(1 - 5*QL_EPSILON)) {
                        if (close_enough(b, B(u)))
                            return 0.5*df;
                        else
                            return df*((b < B(u)? 0.0: 1.0));
                    }
                    else
                        return df*Phi(d(tau - u, b/B(u)).first);
                    }, 0, tau);
            }

            const std::pair<Real, Real> dpm = d(tau, b/K);

            N = Phi(dpm.second) + r*ni;
            D = Phi(dpm.first) + q*di;
        }

        Real fv;
        const Real alpha = K*std::exp(-(r-q)*tau);
        if (tau < squared(QL_EPSILON)) {
            if (close_enough(b, K) || b > K)
                fv = alpha;
            else {
                if (close_enough(q, Real(0)))
                    fv = alpha*r*((q < 0)? -1.0 : 1.0)/QL_EPSILON;
                else
                    fv = alpha*r/q;
            }
        }
        else
            fv = alpha*N/D;

        return std::make_tuple(N, D, fv);
    }

    std::pair<Real, Real> DqFpEquation_B::NDd(Real tau, Real b) const {
        const std::pair<Real, Real> dpm = d(tau, b/K);
        return std::make_pair(
            phi(dpm.second) / (b*vol*std::sqrt(tau)),
            phi(dpm.first)  / (b*vol*std::sqrt(tau))
        );
    }

    QdFpAmericanEngine::QdFpAmericanEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess,
        ext::shared_ptr<QdFpIterationScheme> iterationScheme,
        FixedPointEquation fpEquation)
    : detail::QdPutCallParityEngine(std::move(bsProcess)),
      iterationScheme_(std::move(iterationScheme)),
      fpEquation_(fpEquation) {
    }

    ext::shared_ptr<QdFpIterationScheme>
    QdFpAmericanEngine::fastScheme() {
        static auto scheme = ext::make_shared<QdFpLegendreScheme>(7, 2, 7, 27);
        return scheme;
    }

    ext::shared_ptr<QdFpIterationScheme>
    QdFpAmericanEngine::accurateScheme() {
        static auto scheme = ext::make_shared<QdFpLegendreTanhSinhScheme>(25, 5, 13, 1e-8);
        return scheme;
    }

    ext::shared_ptr<QdFpIterationScheme>
    QdFpAmericanEngine::highPrecisionScheme() {
        static auto scheme = ext::make_shared<QdFpTanhSinhIterationScheme>(10, 30, 1e-10);
        return scheme;
    }

    detail::QdPutResults
    QdFpAmericanEngine::calculatePut(Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const {

        const bool doubleBoundary = (r < 0.0 && q < r);
        const Real tauHat = doubleBoundary ? computeTauHat(r, q, vol, T) : 0.0;
        const Time tauTilde = doubleBoundary ? tauHat : T;

        detail::QdPutResults res;

        const Real xmax =  QdPlusAmericanEngine::xMax(K, r, q);
        const Size n = iterationScheme_->getNumberOfChebyshevInterpolationNodes();

        // --- Lower boundary B: computed over [0, tauTilde] ---
        const ext::shared_ptr<ChebyshevInterpolation> interp =
            QdPlusAmericanEngine(process_, n + 1, QdPlusAmericanEngine::Halley, 1e-8)
                .getPutExerciseBoundary(S, K, r, q, vol, tauTilde);

        const Array z = interp->nodes();
        const Array x = 0.5 * std::sqrt(tauTilde) * (1.0 + z);

        const auto B = [xmax, tauTilde, &interp](Real tau) -> Real {
            const Real z = 2 * std::sqrt(std::abs(tau) / tauTilde) - 1;
            return xmax*std::exp(-std::sqrt(std::max(Real(0), (*interp)(z, true))));
        };

        const auto h = [=](Real fv) -> Real {
            return squared(std::log(fv/xmax));
        };

        const ext::shared_ptr<DqFpEquation> eqn
            = (fpEquation_ == FP_A
               || (fpEquation_ == Auto && std::abs(r-q) < 0.001))?
              ext::shared_ptr<DqFpEquation>(new DqFpEquation_A(
                  K, r, q, vol, B,
                  iterationScheme_->getFixedPointIntegrator()))
            : ext::shared_ptr<DqFpEquation>(new DqFpEquation_B(
                    K, r, q, vol, B,
                    iterationScheme_->getFixedPointIntegrator()));

        Array y(x.size());
        y[0] = 0.0;

        const Size n_newton
            = iterationScheme_->getNumberOfJacobiNewtonFixedPointSteps();
        for (Size k=0; k < n_newton; ++k) {
            for (Size i=1; i < x.size(); ++i) {
                const Real tau = squared(x[i]);
                const Real b = B(tau);

                const std::tuple<Real, Real, Real> results = eqn->f(tau, b);
                const Real N = std::get<0>(results);
                const Real D = std::get<1>(results);
                const Real fv = std::get<2>(results);

                if (tau < QL_EPSILON)
                    y[i] = h(fv);
                else {
                    const std::pair<Real, Real> ndd = eqn->NDd(tau, b);
                    const Real Nd = std::get<0>(ndd);
                    const Real Dd = std::get<1>(ndd);

                    const Real fd = K*std::exp(-(r-q)*tau) * (Nd/D - Dd*N/(D*D));

                    y[i] = h(b - (fv - b)/ (fd-1));
                }
            }
            interp->updateY(y);
        }

        const Size n_fp = iterationScheme_->getNumberOfNaiveFixedPointSteps();
        for (Size k=0; k < n_fp; ++k) {
            for (Size i=1; i < x.size(); ++i) {
                const Real tau = squared(x[i]);
                const Real fv = std::get<2>(eqn->f(tau, B(tau)));

                y[i] = h(fv);
            }
            interp->updateY(y);
        }

        // --- Upper boundary Y (double-boundary case only) ---
        ext::shared_ptr<ChebyshevInterpolation> interpY;
        Real ymax = 0.0;
        std::function<Real(Real)> Y;   // Y(tau) -> upper boundary value
        std::function<Real(Real)> h_y; // h_y(fv) = log(fv/ymax)^2
        ext::shared_ptr<DqFpEquation> eqnY;
        Array xY, yY;

        if (doubleBoundary && tauHat > QL_EPSILON) {
            ymax = K * r / q;
            const Real bAtTauHat = B(tauHat);

            h_y = [ymax](Real fv) -> Real { return squared(std::log(fv / ymax)); };

            // Initial guess: interpolate between Y(0)=ymax and B(tauHat)
            interpY = ext::make_shared<ChebyshevInterpolation>(
                n + 1,
                [&](Real zz) -> Real {
                    const Real tau = 0.25 * tauHat * squared(1 + zz);
                    const Real frac = std::sqrt(tau / tauHat);
                    const Real yGuess = ymax * (1 - frac) + bAtTauHat * frac;
                    return h_y(std::max(QL_EPSILON, yGuess));
                },
                ChebyshevInterpolation::SecondKind);

            Y = [ymax, tauHat, &interpY](Real tau) -> Real {
                const Real z = 2 * std::sqrt(std::abs(tau) / tauHat) - 1;
                return ymax * std::exp(-std::sqrt(std::max(Real(0), (*interpY)(z, true))));
            };

            // FP equation for Y: same DqFpEquation_B structure, Y as boundary
            eqnY = ext::shared_ptr<DqFpEquation>(
                new DqFpEquation_B(K, r, q, vol, Y, iterationScheme_->getFixedPointIntegrator()));

            const Array zY = interpY->nodes();
            xY = 0.5 * std::sqrt(tauHat) * (1.0 + zY);
            yY = Array(xY.size());
            yY[0] = 0.0;

            // Jacobi-Newton steps for Y
            for (Size k = 0; k < n_newton; ++k) {
                for (Size i = 1; i < xY.size(); ++i) {
                    const Real tau = squared(xY[i]);
                    const Real yv = Y(tau);

                    const auto results = eqnY->f(tau, yv);
                    const Real N = std::get<0>(results);
                    const Real D = std::get<1>(results);
                    const Real fv = std::get<2>(results);

                    if (tau < QL_EPSILON)
                        yY[i] = h_y(fv);
                    else {
                        const auto ndd = eqnY->NDd(tau, yv);
                        const Real Nd = std::get<0>(ndd);
                        const Real Dd = std::get<1>(ndd);
                        const Real fd = K * std::exp(-(r - q) * tau) * (Nd / D - Dd * N / (D * D));
                        yY[i] = h_y(yv - (fv - yv) / (fd - 1));
                    }
                }
                interpY->updateY(yY);
            }

            // Naive FP steps for Y
            for (Size k = 0; k < n_fp; ++k) {
                for (Size i = 1; i < xY.size(); ++i) {
                    const Real tau = squared(xY[i]);
                    yY[i] = h_y(std::get<2>(eqnY->f(tau, Y(tau))));
                }
                interpY->updateY(yY);
            }
        }

        const detail::QdPlusAddOnValue aov(T, tauTilde, S, K, r, q, vol, xmax, interp);
        const auto integrator = iterationScheme_->getExerciseBoundaryToPriceIntegrator();

        auto integrate = [&](const auto& f) {
            return (*integrator)(f, std::sqrt(T - tauTilde), std::sqrt(T));
        };

        const Real addOn = integrate(aov);

        // Y add-on: subtract upper boundary contribution.
        // Y exists for time-to-expiry tau in [0, tauHat], i.e. calendar time
        // t in [T-tauHat, T]. Integration variable z = sqrt(t).
        Real addOnY = 0.0;
        if (doubleBoundary && tauHat > QL_EPSILON) {
            const NormalDistribution phiY;
            const CumulativeNormalDistribution PhiY;

            addOnY = (*integrator)(
                [&](Real z) -> Real {
                    const Real t = z * z;
                    const Real tau = T - t; // time-to-expiry
                    if (tau < QL_EPSILON || tau > tauHat + QL_EPSILON)
                        return 0.0;

                    // Evaluate Y at time-to-expiry tau
                    const Real zc = 2 * std::sqrt(std::min(tau, tauHat) / tauHat) - 1;
                    const Real qv = (*interpY)(zc, true);
                    const Real y_t = ymax * std::exp(-std::sqrt(std::max(0.0, qv)));

                    const Real dr_ = std::exp(-r * t);
                    const Real dq_ = std::exp(-q * t);
                    const Real v_ = vol * std::sqrt(t);

                    if (v_ < QL_EPSILON || y_t < QL_EPSILON)
                        return 0.0;

                    const Real dp_ = std::log(S * dq_ / (y_t * dr_)) / v_ + 0.5 * v_;
                    return 2 * z * (r * K * dr_ * PhiY(-(dp_ - v_)) - q * S * dq_ * PhiY(-dp_));
                },
                std::sqrt(std::max(0.0, T - tauHat)), std::sqrt(T));
        }

        const Real fwd = S * std::exp((r - q) * T);
        const Real stdDev = vol * std::sqrt(T);
        const Real df = std::exp(-r * T);
        BlackCalculator bc(Option::Put, K, fwd, stdDev, df);

        // Pricing formula depends on S location (paper step 6):
        // S > B(0)=xmax: eq(15), only B add-on
        // S < Y(0)=ymax: eq(16), V = K - S - addOnY
        // Otherwise: eq(12) with both boundaries, or exercise
        if (doubleBoundary && tauHat > QL_EPSILON) {
            // Check if S is in the immediate exercise region at tau=T (now).
            // Boundaries only exist for tau <= tauHat; at tau=T the exercise
            // region is [Y(T), B(T)] if T <= tauHat, otherwise empty.
            if (T <= tauHat) {
                const Real B_T = B(T);
                const Real Y_T = Y(T);
                if (S >= Y_T && S <= B_T) {
                    res.value = K - S;
                    res.delta = -1.0;
                    // gamma, vega, rho, divRho, theta all zero
                    res.strikeSensitivity = 1.0;
                    return res;
                }
            }
            if (S >= xmax) {
                // Eq (15): upper continuation region, only B
                res.value = std::max(0.0, bc.value()) + std::max(0.0, addOn);
            } else if (S <= ymax) {
                // Eq (16): lower continuation region
                res.value = std::max(0.0, K - S - addOnY);
            } else {
                // Between ymax and xmax but outside [Y(T), B(T)]
                res.value = std::max(0.0, bc.value()) + std::max(0.0, addOn - addOnY);
            }
        } else {
            res.value = std::max(0.0, bc.value()) + std::max(0.0, addOn - addOnY);
        }

        const NormalDistribution phi;
        const CumulativeNormalDistribution Phi;

        // Delta add-on
        const Real deltaAddOn = integrate([&](Real z) -> Real {
            const QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *interp);
            if (!s.valid)
                return 0.0;
            return 2 * z *
                   (-r * K * s.dr * phi(s.dm) / (S * s.v) - q * s.dq * Phi(-s.dp) +
                    q * s.dq * phi(s.dp) / s.v);
        });
        res.delta = bc.delta(S) + deltaAddOn;

        // Gamma add-on
        const Real gammaAddOn = integrate([&](Real z) -> Real {
            const QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *interp);
            if (!s.valid)
                return 0.0;
            return 2 * z *
                   (r * K * s.dr * phi(s.dm) * s.dp / (S * S * s.v * s.v) -
                    q * s.dq * phi(s.dp) * s.dm / (S * s.v * s.v));
        });
        res.gamma = bc.gamma(S) + gammaAddOn;

        // --- Y boundary contributions to delta, gamma (double-boundary) ---
        // The Y add-on uses the same integrand form as B but with the upper
        // boundary y_t, and is subtracted from the total.
        // Y boundary setup: evaluates Y at integration point z
        auto yBoundarySetup =
            [&](Real z) -> std::tuple<bool, Real, Real, Real, Real, Real, Real, Real> {
            const Real t = z * z;
            const Real tau = T - t;
            if (tau < QL_EPSILON || tau > tauHat + QL_EPSILON)
                return {false, 0, 0, 0, 0, 0, 0, 0};
            const Real zc = 2 * std::sqrt(std::min(tau, tauHat) / tauHat) - 1;
            const Real qv = (*interpY)(zc, true);
            const Real y_t = ymax * std::exp(-std::sqrt(std::max(0.0, qv)));
            const Real dr_ = std::exp(-r * t);
            const Real dq_ = std::exp(-q * t);
            const Real v_ = vol * std::sqrt(t);
            if (v_ < QL_EPSILON || y_t < QL_EPSILON)
                return {false, 0, 0, 0, 0, 0, 0, 0};
            const Real dp_ = std::log(S * dq_ / (y_t * dr_)) / v_ + 0.5 * v_;
            const Real dm_ = dp_ - v_;
            return {true, t, dr_, dq_, v_, y_t, dp_, dm_};
        };

        const Real zYlo = std::sqrt(std::max(0.0, T - tauHat));
        const Real zYhi = std::sqrt(T);
        auto integrateY = [&](const auto& f) -> Real {
            if (!doubleBoundary || tauHat <= QL_EPSILON)
                return 0.0;
            return (*integrator)(f, zYlo, zYhi);
        };

        if (doubleBoundary && tauHat > QL_EPSILON) {
            const Real deltaYAddOn = integrateY([&](Real z) -> Real {
                const auto [valid, t, dr_, dq_, v_, y_t, dp_, dm_] = yBoundarySetup(z);
                if (!valid)
                    return 0.0;
                return 2 * z *
                       (-r * K * dr_ * phi(dm_) / (S * v_) - q * dq_ * Phi(-dp_) +
                        q * dq_ * phi(dp_) / v_);
            });
            res.delta -= deltaYAddOn;

            const Real gammaYAddOn = integrateY([&](Real z) -> Real {
                const auto [valid, t, dr_, dq_, v_, y_t, dp_, dm_] = yBoundarySetup(z);
                if (!valid)
                    return 0.0;
                return 2 * z *
                       (r * K * dr_ * phi(dm_) * dp_ / (S * S * v_ * v_) -
                        q * dq_ * phi(dp_) * dm_ / (S * v_ * v_));
            });
            res.gamma -= gammaYAddOn;
        }

        // Analytical vega, rho, dividendRho via tangent linear FP iteration.
        // dV/dp = dV/dp|_B + integral (df/dB)*(dB/dp) dz
        // where dB/dp is obtained by differentiating the FP equation.
        {
            const Array y_orig(y);
            const Size nNodes = x.size();
            const Size m = nNodes - 1;

            // Determine FP equation type consistently
            const bool useFpA =
                (fpEquation_ == FP_A || (fpEquation_ == Auto && std::abs(r - q) < 0.001));

            // Use GaussLegendre for sensitivity equations to avoid
            // GaussLobatto max-iteration failures with bumped parameters
            auto makeSensEqn = [&](Rate r_, Rate q_,
                                   Volatility vol_) -> ext::shared_ptr<DqFpEquation> {
                auto sensInteg = ext::make_shared<GaussLegendreIntegrator>(25);
                if (useFpA)
                    return ext::shared_ptr<DqFpEquation>(
                        new DqFpEquation_A(K, r_, q_, vol_, B, std::move(sensInteg)));
                else
                    return ext::shared_ptr<DqFpEquation>(
                        new DqFpEquation_B(K, r_, q_, vol_, B, std::move(sensInteg)));
            };

            // Compute y_fp = h(G(B;p)) explicitly. This differs from y_orig
            // by the FP residual; using y_orig in finite differences would
            // amplify this residual by 1/eps, corrupting the sensitivities.
            Array y_fp(nNodes);
            y_fp[0] = 0.0;
            for (Size i = 1; i < nNodes; ++i) {
                const Real tau = squared(x[i]);
                y_fp[i] = h(std::get<2>(eqn->f(tau, B(tau))));
            }

            // Step 1: dG/dp -- central difference for O(eps^2) accuracy
            const Real eps_sigma = vol * 1e-4;
            const Real eps_r = std::max(std::abs(r), 1.0) * 1e-4;
            const Real eps_q = std::max(std::abs(q), 1.0) * 1e-4;

            const auto eqn_sv_up = makeSensEqn(r, q, vol + eps_sigma);
            const auto eqn_sv_dn = makeSensEqn(r, q, vol - eps_sigma);
            const auto eqn_rv_up = makeSensEqn(r + eps_r, q, vol);
            const auto eqn_rv_dn = makeSensEqn(r - eps_r, q, vol);
            const auto eqn_qv_up = makeSensEqn(r, q + eps_q, vol);
            const auto eqn_qv_dn = makeSensEqn(r, q - eps_q, vol);

            Array g_sigma(m), g_r(m), g_q(m);
            for (Size i = 1; i < nNodes; ++i) {
                const Real tau = squared(x[i]);
                const Real b = B(tau);
                g_sigma[i - 1] =
                    (h(std::get<2>(eqn_sv_up->f(tau, b))) - h(std::get<2>(eqn_sv_dn->f(tau, b)))) /
                    (2 * eps_sigma);
                g_r[i - 1] =
                    (h(std::get<2>(eqn_rv_up->f(tau, b))) - h(std::get<2>(eqn_rv_dn->f(tau, b)))) /
                    (2 * eps_r);
                g_q[i - 1] =
                    (h(std::get<2>(eqn_qv_up->f(tau, b))) - h(std::get<2>(eqn_qv_dn->f(tau, b)))) /
                    (2 * eps_q);
            }

            // Step 2: Sensitivity FP iteration -- same contraction as boundary
            Array s_sigma(g_sigma), s_r(g_r), s_q(g_q);
            const Real eps_fd = std::sqrt(QL_EPSILON);

            auto iterateSens = [&](Array& s_p, const Array& g_p) {
                Array y_pert(y_orig);
                for (Size i = 1; i < nNodes; ++i)
                    y_pert[i] = y_orig[i] + eps_fd * s_p[i - 1];
                interp->updateY(y_pert);

                for (Size i = 1; i < nNodes; ++i) {
                    const Real tau = squared(x[i]);
                    const Real b = B(tau);
                    s_p[i - 1] = g_p[i - 1] + (h(std::get<2>(eqn->f(tau, b))) - y_fp[i]) / eps_fd;
                }
                interp->updateY(y_orig);
            };

            for (Size k = 0; k < n_fp; ++k) {
                iterateSens(s_sigma, g_sigma);
                iterateSens(s_r, g_r);
                iterateSens(s_q, g_q);
            }

            // Step 3: Convert dy/dp -> dB/dp at nodes, then interpolate dB/dp
            // dB = B * (-1/(2*sqrt(y))) * dy avoids amplification by interpolating
            // the smooth function dB/dp directly.
            auto makeBoundarySensInterp = [&](const Array& s_p) {
                Array db_dp(nNodes);
                db_dp[0] = 0.0;
                for (Size i = 1; i < nNodes; ++i) {
                    const Real yi = y_orig[i];
                    if (yi < QL_EPSILON) {
                        db_dp[i] = 0.0;
                    } else {
                        const Real Bi = B(squared(x[i]));
                        db_dp[i] = Bi * (-1.0 / (2 * std::sqrt(yi))) * s_p[i - 1];
                    }
                }
                return ext::make_shared<ChebyshevInterpolation>(db_dp,
                                                                ChebyshevInterpolation::SecondKind);
            };

            const auto si_sigma = makeBoundarySensInterp(s_sigma);
            const auto si_r = makeBoundarySensInterp(s_r);
            const auto si_q = makeBoundarySensInterp(s_q);

            // When tauHat < T, dB/dp has two parts:
            //  (a) FP sensitivity at fixed tauHat (from the tangent-linear iteration)
            //  (b) grid remapping: the Chebyshev coordinate z_c = 2*sqrt(tau/tauHat)-1
            //      shifts when tauHat changes with p.
            //      dB_remap/dp = (dB/dz_c)*(dz_c/dtauHat)*(dtauHat/dp)
            // For vega, |dtauHat/dsigma| is small so (b) is negligible.
            // For rho/divRho, |dtauHat/dr| ~= 30x|dtauHat/dsigma| so (b) is significant.

            // Compute tauHat sensitivities (needed for grid remapping and Leibniz)
            const bool hasMerge = doubleBoundary && tauTilde < T - QL_EPSILON;
            TauHatSensitivities ths{0, 0, 0};
            if (hasMerge)
                ths = computeTauHatSensitivities(r, q, tauHat);

            // B boundary: FP sensitivity lookup
            auto lookupDBdp = [&](Real t_val, const ChebyshevInterpolation& si) -> Real {
                const Real tau = T - t_val;
                const Real zc = 2 * std::sqrt(std::max(0.0, tau / tauTilde)) - 1;
                return si(zc, true);
            };

            // Vega add-on
            const Real vegaAddOn = integrate([&](Real z) -> Real {
                const QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *interp);
                if (!s.valid)
                    return 0.0;
                const Real dBds = lookupDBdp(s.t, *si_sigma);
                const Real ddp_ds = -s.dm / vol - dBds / (s.b_t * s.v);
                const Real ddm_ds = -s.dp / vol - dBds / (s.b_t * s.v);
                return 2 * z *
                       (-r * K * s.dr * phi(s.dm) * ddm_ds + q * S * s.dq * phi(s.dp) * ddp_ds);
            });
            res.vega = bc.vega(T) + vegaAddOn;

            // Rho add-on
            const Real rhoAddOn = integrate([&](Real z) -> Real {
                const QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *interp);
                if (!s.valid)
                    return 0.0;
                const Real dBdr = lookupDBdp(s.t, *si_r);
                const Real ddp_dr = s.t / s.v - dBdr / (s.b_t * s.v);
                return 2 * z *
                       ((1 - r * s.t) * K * s.dr * Phi(-s.dm) - r * K * s.dr * phi(s.dm) * ddp_dr +
                        q * S * s.dq * phi(s.dp) * ddp_dr);
            });
            res.rho = bc.rho(T) + rhoAddOn;

            // DividendRho add-on
            const Real divRhoAddOn = integrate([&](Real z) -> Real {
                const QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *interp);
                if (!s.valid)
                    return 0.0;
                const Real dBdq = lookupDBdp(s.t, *si_q);
                const Real ddp_dq = -s.t / s.v - dBdq / (s.b_t * s.v);
                return 2 * z *
                       (-r * K * s.dr * phi(s.dm) * ddp_dq - (1 - q * s.t) * S * s.dq * Phi(-s.dp) +
                        q * S * s.dq * phi(s.dp) * ddp_dq);
            });
            res.dividendRho = bc.dividendRho(T) + divRhoAddOn;

            // --- Y boundary sensitivity for vega/rho/divRho ---
            if (doubleBoundary && tauHat > QL_EPSILON) {
                const Array yY_orig(yY);
                const Size nYNodes = xY.size();
                const Size mY = nYNodes - 1;

                // Y FP baseline values
                Array y_fp_Y(nYNodes);
                y_fp_Y[0] = 0.0;
                for (Size i = 1; i < nYNodes; ++i) {
                    const Real tau = squared(xY[i]);
                    y_fp_Y[i] = h_y(std::get<2>(eqnY->f(tau, Y(tau))));
                }

                // Bumped FP equations for Y (always use FP_B for Y boundary)
                auto makeSensEqnY = [&](Rate r_, Rate q_,
                                        Volatility vol_) -> ext::shared_ptr<DqFpEquation> {
                    auto sensInteg = ext::make_shared<GaussLegendreIntegrator>(25);
                    return ext::shared_ptr<DqFpEquation>(
                        new DqFpEquation_B(K, r_, q_, vol_, Y, std::move(sensInteg)));
                };

                const auto eqnY_sv_up = makeSensEqnY(r, q, vol + eps_sigma);
                const auto eqnY_sv_dn = makeSensEqnY(r, q, vol - eps_sigma);
                const auto eqnY_rv_up = makeSensEqnY(r + eps_r, q, vol);
                const auto eqnY_rv_dn = makeSensEqnY(r - eps_r, q, vol);
                const auto eqnY_qv_up = makeSensEqnY(r, q + eps_q, vol);
                const auto eqnY_qv_dn = makeSensEqnY(r, q - eps_q, vol);

                // dG_Y/dp via central difference
                Array gY_sigma(mY), gY_r(mY), gY_q(mY);
                for (Size i = 1; i < nYNodes; ++i) {
                    const Real tau = squared(xY[i]);
                    const Real yt = Y(tau);
                    gY_sigma[i - 1] = (h_y(std::get<2>(eqnY_sv_up->f(tau, yt))) -
                                       h_y(std::get<2>(eqnY_sv_dn->f(tau, yt)))) /
                                      (2 * eps_sigma);
                    gY_r[i - 1] = (h_y(std::get<2>(eqnY_rv_up->f(tau, yt))) -
                                   h_y(std::get<2>(eqnY_rv_dn->f(tau, yt)))) /
                                  (2 * eps_r);
                    gY_q[i - 1] = (h_y(std::get<2>(eqnY_qv_up->f(tau, yt))) -
                                   h_y(std::get<2>(eqnY_qv_dn->f(tau, yt)))) /
                                  (2 * eps_q);
                }

                // Sensitivity FP iteration for Y
                Array sY_sigma(gY_sigma), sY_r(gY_r), sY_q(gY_q);

                auto iterateSensY = [&](Array& s_p, const Array& g_p) {
                    Array yY_pert(yY_orig);
                    for (Size i = 1; i < nYNodes; ++i)
                        yY_pert[i] = yY_orig[i] + eps_fd * s_p[i - 1];
                    interpY->updateY(yY_pert);

                    for (Size i = 1; i < nYNodes; ++i) {
                        const Real tau = squared(xY[i]);
                        const Real yt = Y(tau);
                        s_p[i - 1] =
                            g_p[i - 1] + (h_y(std::get<2>(eqnY->f(tau, yt))) - y_fp_Y[i]) / eps_fd;
                    }
                    interpY->updateY(yY_orig);
                };

                for (Size k = 0; k < n_fp; ++k) {
                    iterateSensY(sY_sigma, gY_sigma);
                    iterateSensY(sY_r, gY_r);
                    iterateSensY(sY_q, gY_q);
                }

                // Convert dy_Y/dp -> dY/dp
                auto makeYBoundarySensInterp = [&](const Array& s_p) {
                    Array dy_dp(nYNodes);
                    dy_dp[0] = 0.0;
                    for (Size i = 1; i < nYNodes; ++i) {
                        const Real yi = yY_orig[i];
                        if (yi < QL_EPSILON) {
                            dy_dp[i] = 0.0;
                        } else {
                            const Real Yi = Y(squared(xY[i]));
                            dy_dp[i] = Yi * (-1.0 / (2 * std::sqrt(yi))) * s_p[i - 1];
                        }
                    }
                    return ext::make_shared<ChebyshevInterpolation>(
                        dy_dp, ChebyshevInterpolation::SecondKind);
                };

                const auto siY_sigma = makeYBoundarySensInterp(sY_sigma);
                const auto siY_r = makeYBoundarySensInterp(sY_r);
                const auto siY_q = makeYBoundarySensInterp(sY_q);

                // Y boundary: FP sensitivity lookup
                auto lookupDYdp = [&](Real t_val, const ChebyshevInterpolation& si) -> Real {
                    const Real tau = T - t_val;
                    const Real zc =
                        2 * std::sqrt(std::max(0.0, std::min(tau, tauHat)) / tauHat) - 1;
                    return si(zc, true);
                };

                // Vega Y add-on (subtracted)
                const Real vegaYAddOn = integrateY([&](Real z) -> Real {
                    const auto [valid, t, dr_, dq_, v_, y_t, dp_, dm_] = yBoundarySetup(z);
                    if (!valid)
                        return 0.0;
                    const Real dYds = lookupDYdp(t, *siY_sigma);
                    const Real ddp_ds = -dm_ / vol - dYds / (y_t * v_);
                    const Real ddm_ds = -dp_ / vol - dYds / (y_t * v_);
                    return 2 * z *
                           (-r * K * dr_ * phi(dm_) * ddm_ds + q * S * dq_ * phi(dp_) * ddp_ds);
                });
                res.vega -= vegaYAddOn;

                // Rho Y add-on (subtracted)
                const Real rhoYAddOn = integrateY([&](Real z) -> Real {
                    const auto [valid, t, dr_, dq_, v_, y_t, dp_, dm_] = yBoundarySetup(z);
                    if (!valid)
                        return 0.0;
                    const Real dYdr = lookupDYdp(t, *siY_r);
                    const Real ddp_dr = t / v_ - dYdr / (y_t * v_);
                    return 2 * z *
                           ((1 - r * t) * K * dr_ * Phi(-dm_) - r * K * dr_ * phi(dm_) * ddp_dr +
                            q * S * dq_ * phi(dp_) * ddp_dr);
                });
                res.rho -= rhoYAddOn;

                // DividendRho Y add-on (subtracted)
                const Real divRhoYAddOn = integrateY([&](Real z) -> Real {
                    const auto [valid, t, dr_, dq_, v_, y_t, dp_, dm_] = yBoundarySetup(z);
                    if (!valid)
                        return 0.0;
                    const Real dYdq = lookupDYdp(t, *siY_q);
                    const Real ddp_dq = -t / v_ - dYdq / (y_t * v_);
                    return 2 * z *
                           (-r * K * dr_ * phi(dm_) * ddp_dq - (1 - q * t) * S * dq_ * Phi(-dp_) +
                            q * S * dq_ * phi(dp_) * ddp_dq);
                });
                res.dividendRho -= divRhoYAddOn;
            }

            // --- Leibniz boundary correction for vega/rho/divRho ---
            // When tauHat < T, the integration lower limit z0 = sqrt(T-tauHat) depends on
            // sigma, r, q through tauHat. By Leibniz rule: correction =
            // [f_B(z0)-f_Y(z0)]*dtauHat/dp/(2z0)
            if (doubleBoundary && tauTilde < T - QL_EPSILON) {
                const Real z0 = std::sqrt(T - tauHat);
                const Real fB_z0 = aov(z0);

                // Evaluate Y integrand at z0
                Real fY_z0 = 0.0;
                if (interpY) {
                    const Real t0 = z0 * z0;
                    const Real tau0 = T - t0;
                    const Real zc = 2 * std::sqrt(std::min(tau0, tauHat) / tauHat) - 1;
                    const Real qv = (*interpY)(zc, true);
                    const Real y0 = ymax * std::exp(-std::sqrt(std::max(0.0, qv)));
                    const Real dr0 = std::exp(-r * t0);
                    const Real dq0 = std::exp(-q * t0);
                    const Real v0 = vol * std::sqrt(t0);
                    if (v0 >= QL_EPSILON && y0 > QL_EPSILON) {
                        const Real dp0 = std::log(S * dq0 / (y0 * dr0)) / v0 + 0.5 * v0;
                        fY_z0 = 2 * z0 * (r * K * dr0 * Phi(-(dp0 - v0)) - q * S * dq0 * Phi(-dp0));
                    }
                }

                const Real fNet = fB_z0 - fY_z0;
                if (std::abs(fNet) > QL_EPSILON) {
                    const Real leibnizFactor = fNet / (2 * z0);
                    res.vega += leibnizFactor * ths.dTauHat_dSigma;
                    res.rho += leibnizFactor * ths.dTauHat_dR;
                    res.dividendRho += leibnizFactor * ths.dTauHat_dQ;
                }
            }
        }

        // StrikeSensitivity add-on: total d/dK, using B proportional to K
        // dp = [log(S/K) - log(g) + (r-q)t]/v + v/2, so ddp/dK = -1/(Kv)
        const Real strikeSensAddOn = integrate([&](Real z) -> Real {
            const QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *interp);
            if (!s.valid)
                return 0.0;
            return 2 * z *
                   (r * s.dr * (Phi(-s.dm) + phi(s.dm) / s.v) -
                    q * S * s.dq * phi(s.dp) / (K * s.v));
        });
        res.strikeSensitivity = bc.strikeSensitivity() + strikeSensAddOn;

        // StrikeGamma add-on: d^2f/dK^2
        const Real strikeGammaAddOn = integrate([&](Real z) -> Real {
            const QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *interp);
            if (!s.valid)
                return 0.0;
            return 2 * z *
                   (r * s.dr * phi(s.dm) * s.dp / (K * s.v * s.v) -
                    q * S * s.dq * phi(s.dp) * s.dm / (K * K * s.v * s.v));
        });
        res.strikeGamma = bc.strikeGamma() + strikeGammaAddOn;

        // --- Y boundary contributions to strikeSens, strikeGamma ---
        // Y = K*r/q proportional to K, so dY/dK = r/q. The Y integrand also depends on K
        // directly through the rK*dr*Phi(-d-) term. ddp/dK = -1/(K*v).
        if (doubleBoundary && tauHat > QL_EPSILON) {
            const Real strikeSensYAddOn = integrateY([&](Real z) -> Real {
                const auto [valid, t, dr_, dq_, v_, y_t, dp_, dm_] = yBoundarySetup(z);
                if (!valid)
                    return 0.0;
                return 2 * z *
                       (r * dr_ * (Phi(-dm_) + phi(dm_) / v_) - q * S * dq_ * phi(dp_) / (K * v_));
            });
            res.strikeSensitivity -= strikeSensYAddOn;

            const Real strikeGammaYAddOn = integrateY([&](Real z) -> Real {
                const auto [valid, t, dr_, dq_, v_, y_t, dp_, dm_] = yBoundarySetup(z);
                if (!valid)
                    return 0.0;
                return 2 * z *
                       (r * dr_ * phi(dm_) * dp_ / (K * v_ * v_) -
                        q * S * dq_ * phi(dp_) * dm_ / (K * K * v_ * v_));
            });
            res.strikeGamma -= strikeGammaYAddOn;
        }

        // Theta: Leibniz rule -> f(sqrt(T)) / (2*sqrt(T))
        const Real sqrtT = std::sqrt(T);
        res.theta = bc.theta(S, T) + aov(sqrtT) / (2 * sqrtT);

        return res;
    }
}




