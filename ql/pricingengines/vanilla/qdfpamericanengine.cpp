/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Klaus Spanderen

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

/*! \file qrfpamericanengine.cpp
*/

#include <ql/math/functional.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/interpolations/chebyshevinterpolation.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/vanilla/qdfpamericanengine.hpp>
#include <ql/math/integrals/tanhsinhintegral.hpp>
#ifndef QL_BOOST_HAS_TANH_SINH
#include <ql/math/integrals/gausslobattointegral.hpp>
#endif

#include <iostream>

namespace QuantLib {
    QdFpLegendreScheme::QdFpLegendreScheme(
        Size l, Size m, Size n, Size p):
        m_(m), n_(n),
        fpIntegrator_(ext::make_shared<GaussLegendreIntegrator>(l)),
        exerciseBoundaryIntegrator_(
            ext::make_shared<GaussLegendreIntegrator>(p)) {

        QL_REQUIRE(m_ > 0, "at least one fixed point iteration step is needed");
        QL_REQUIRE(n_ > 0, "at least one interpolation points is needed");
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

    ext::shared_ptr<QdFpIterationScheme>
        QdFpIterationSchemeStdFactory::fastScheme_
            = ext::make_shared<QdFpLegendreScheme>(7, 2, 6, 15);

    ext::shared_ptr<QdFpIterationScheme>
    QdFpIterationSchemeStdFactory::fastScheme() {
        return QdFpIterationSchemeStdFactory::fastScheme_;
    }

    ext::shared_ptr<QdFpIterationScheme>
        QdFpIterationSchemeStdFactory::accurateScheme_
            = ext::make_shared<QdFpLegendreTanhSinhScheme>(25, 3, 13, 1e-8);

    ext::shared_ptr<QdFpIterationScheme>
    QdFpIterationSchemeStdFactory::accurateScheme() {
        return QdFpIterationSchemeStdFactory::accurateScheme_;
    }

    ext::shared_ptr<QdFpIterationScheme>
    QdFpIterationSchemeStdFactory::highPrecisionScheme() {
        return QdFpIterationSchemeStdFactory::highPrecisionScheme_;
    }

    ext::shared_ptr<QdFpIterationScheme>
        QdFpIterationSchemeStdFactory::highPrecisionScheme_
            = ext::make_shared<QdFpTanhSinhIterationScheme>(8, 32, 1e-10);


    class DqFpEquation {
      public:
        DqFpEquation(Rate _r, Rate _q, Volatility _vol)
        : r(_r), q(_q), vol(_vol) {}

        virtual std::pair<Real, Real> NDd(Real tau, Real b) const = 0;
        virtual std::tuple<Real, Real, Real> f(Real tau, Real b) const = 0;

        virtual ~DqFpEquation() = default;

      protected:
        std::pair<Real, Real> d(Time t, Real z) const {
            const Real v = vol * std::sqrt(t);
            const Real m = (std::log(z) + (r-q)*t)/v + 0.5*v;

            return std::make_pair(m, m-v);
        }

        const Rate r, q;
        const Volatility vol;

        const NormalDistribution phi;
        const CumulativeNormalDistribution Phi;
    };

    class DqFpEquation_B: public DqFpEquation {
      public:
        DqFpEquation_B(
            Real _S, Real _K, Rate _r, Rate _q,
            Volatility _vol, Time _T,
            std::function<Real(Real)> _B,
            ext::shared_ptr<Integrator> _integrator);

        std::pair<Real, Real> NDd(Real tau, Real b) const override;
        std::tuple<Real, Real, Real> f(Real tau, Real b) const override;

      private:
          const Real S, K;
          const Time T;
          const std::function<Real(Real)> B;
          const ext::shared_ptr<Integrator> integrator;
    };

    class DqFpEquation_A: public DqFpEquation {
      public:
        DqFpEquation_A(
            Real _S, Real _K, Rate _r, Rate _q,
            Volatility _vol, Time _T,
            std::function<Real(Real)> _B,
            ext::shared_ptr<Integrator> _integrator);

        std::pair<Real, Real> NDd(Real tau, Real b) const override;
        std::tuple<Real, Real, Real> f(Real tau, Real b) const override;

      private:
          const Real S, K;
          const Time T;
          const std::function<Real(Real)> B;
          const ext::shared_ptr<Integrator> integrator;
    };

    DqFpEquation_A::DqFpEquation_A(
        Real _S, Real _K, Rate _r, Rate _q, Volatility _vol, Time _T,
        std::function<Real(Real)> _B,
        ext::shared_ptr<Integrator> _integrator)
    : DqFpEquation(_r, _q, _vol),
      S(_S), K(_K), T(_T),
      B(std::move(_B)),
      integrator(_integrator) {
    }

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
            const auto K12 = [&, this](Real tau)
                -> Real {
                const Real stv = std::sqrt(tau)/vol;

                return (*integrator)([=](Real y) -> Real {
                    const Real m = 0.25*tau*squared(1+y);
                    const Real dp =
                        d(0.25*tau*squared(1+y), b/B(tau-m)).first;

                    return std::exp(q*tau - q*m)
                        *(0.5*tau*(y+1)*Phi(dp) + stv*phi(dp));
                }, -1, 1);
            };

            const auto K3 = [&, this](Real tau)
                -> Real {
                const Real stv = std::sqrt(tau)/vol;

                return (*integrator)([=](Real y) -> Real {
                    const Real m = 0.25*tau*squared(1+y);

                    return stv*std::exp(r*tau-r*m)*phi(
                        d(0.25*tau*squared(1+y), b/B(tau-m)).second);
                }, -1, 1);
            };


            const std::pair<Real, Real> dpm = d(tau, b/K);
            N = phi(dpm.second)/v + r*K3(tau);
            D = phi(dpm.first)/v + Phi(dpm.first) + q*K12(tau);
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


    DqFpEquation_B::DqFpEquation_B(
        Real _S, Real _K, Rate _r, Rate _q, Volatility _vol, Time _T,
        std::function<Real(Real)> _B,
        ext::shared_ptr<Integrator> _integrator)
    : DqFpEquation(_r, _q, _vol),
      S(_S), K(_K), T(_T),
      B(std::move(_B)),
      integrator(_integrator) {
    }


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
            const std::pair<Real, Real> dpm = d(tau, b/K);

            N = Phi(dpm.second)
                + r*(*integrator)([=](Real u) -> Real {
                    return std::exp(r*u)*Phi(d(tau - u, b/B(u)).second);
                }, 0, tau);

            D = Phi(dpm.first)
                + q*(*integrator)([=](Real u) -> Real {
                    return std::exp(q*u)*Phi(d(tau - u, b/B(u)).first);
                }, 0, tau);
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

    Real QdFpAmericanEngine::calculatePut(
            Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const {

        const Real xmax =  QdPlusAmericanEngine::xMax(K, r, q);
        const Size n = iterationScheme_->getNumberOfChebyshevInterpolationNodes();

        const ext::shared_ptr<ChebyshevInterpolation> interp =
            QdPlusAmericanEngine(process_, n+1, QdPlusAmericanEngine::Halley, 1e-8)
                .getPutExerciseBoundary(S, K, r, q, vol, T);

        const Array z = interp->nodes();
        const Array x = 0.5*std::sqrt(T)*(1.0+z);

        const auto B = [xmax, T, &interp](Real tau) -> Real {
            const Real z = 2*std::sqrt(std::abs(tau)/T)-1;
            return xmax*std::exp(-std::sqrt(std::max(Real(0), (*interp)(z, true))));
        };

        const auto h = [=](Real fv) -> Real {
            return squared(std::log(fv/xmax));
        };

        const ext::shared_ptr<DqFpEquation> eqn
            = (fpEquation_ == FP_A
               || (fpEquation_ == Auto && std::abs(r-q) < 0.001))?
              ext::shared_ptr<DqFpEquation>(new DqFpEquation_A(
                  S, K, r, q, vol, T, B,
                  iterationScheme_->getFixedPointIntegrator()))
            : ext::shared_ptr<DqFpEquation>(new DqFpEquation_B(
                    S, K, r, q, vol, T, B,
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

        const detail::QdPlusAddOnValue aov(T, S, K, r, q, vol, xmax, interp);
        const Real addOn =
           (*iterationScheme_->getExerciseBoundaryToPriceIntegrator())(
               aov, 0.0, std::sqrt(T));

        const Real europeanValue = BlackCalculator(
            Option::Put, K, S*std::exp((r-q)*T),
            vol*std::sqrt(T), std::exp(-r*T)).value();

        return std::max(europeanValue, 0.0) + std::max(0.0, addOn);
    }



//    Real QdFpAmericanEngine::calculatePutFP_A(
//        Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const {
//
//        const Real xmax =  QdPlusAmericanEngine::xMax(K, r, q);
//        const Size n = iterationScheme_->getNumberOfChebyshevInterpolationNodes();
//
//        ext::shared_ptr<ChebyshevInterpolation> interp =
//            QdPlusAmericanEngine(process_, n+1, QdPlusAmericanEngine::Halley, 1e-8)
//                .getPutExerciseBoundary(S, K, r, q, vol, T);
//
//        const Array z = interp->nodes();
//        const Array x = 0.5*std::sqrt(T)*(1.0+z);
//
//        const auto B = [xmax, T](
//            Real tau, const ext::shared_ptr<Interpolation>& interp) -> Real {
//            const Real z = 2*std::sqrt(std::abs(tau)/T)-1;
//            return xmax*std::exp(-std::sqrt(std::max(Real(0), (*interp)(z, true))));
//        };
//
//        const NormalDistribution phi;
//        const CumulativeNormalDistribution Phi;
//
//        const auto d = [=](Time t, Real z) -> std::pair<Real, Real>  {
//            const Real v = vol * std::sqrt(t);
//            const Real m = (std::log(z) + (r-q)*t)/v + 0.5*v;
//
//            return std::make_pair(m, m-v);
//        };
//
//        const auto K12 = [&, this](Real tau, const ext::shared_ptr<Interpolation>& interp)
//            -> Real {
//            const Real stv = std::sqrt(tau)/vol;
//            const Real b_tau = B(tau, interp);
//
//            return (*iterationScheme_->getFixedPointIntegrator())(
//                [=](Real y) -> Real {
//
//                    const Real m = 0.25*tau*squared(1+y);
//                    const Real dp =
//                        d(0.25*tau*squared(1+y),
//                                b_tau/B(tau-m, interp)).first;
//
//                    return std::exp(q*tau - q*m)
//                        *(0.5*tau*(y+1)*Phi(dp) + stv*phi(dp));
//                }, -1, 1);
//        };
//
//        const auto K3 = [&, this](Real tau, const ext::shared_ptr<Interpolation>& interp)
//            -> Real {
//            const Real stv = std::sqrt(tau)/vol;
//            const Real b_tau = B(tau, interp);
//
//            return (*iterationScheme_->getFixedPointIntegrator())(
//                [=](Real y) -> Real {
//                    const Real m = 0.25*tau*squared(1+y);
//
//                    return stv*std::exp(r*tau-r*m)*phi(
//                        d(0.25*tau*squared(1+y), b_tau/B(tau-m, interp)).second);
//                }, -1, 1);
//        };
//
//        const auto ND = [=](
//            Real tau, Real b, const ext::shared_ptr<Interpolation>& interp)
//            -> std::pair<Real, Real> {
//
//            const Real v = vol * std::sqrt(tau);
//
//            Real N, D;
//            if (tau < squared(QL_EPSILON)) {
//                if (close_enough(b, K)) {
//                    N = 1/(M_SQRT2*M_SQRTPI * v);
//                    D = N + 0.5;
//                }
//                else {
//                    N = 0.0;
//                    D = (b > K)? 1.0: 0.0;
//                }
//            }
//            else {
//                const std::pair<Real, Real> dpm = d(tau, b/K);
//                N = phi(dpm.second)/v + r*K3(tau, interp);
//                D = phi(dpm.first)/v + Phi(dpm.first) + q*K12(tau, interp);
//            }
//
//            return std::make_pair(N, D);
//        };
//
//        const auto f = [=](
//            Real tau, Real b, const ext::shared_ptr<Interpolation>& interp) -> Real {
//            const Real alpha = K*std::exp(-(r-q)*tau);
//
//            if (tau < squared(QL_EPSILON)) {
//                if (close_enough(b, K))
//                    return alpha;
//                else if (b > K)
//                    return 0.0;
//                else {
//                    if (close_enough(q, Real(0)))
//                        return alpha*r*((q < 0)? -1.0 : 1.0)/QL_EPSILON;
//                    else
//                        return alpha*r/q;
//                }
//            }
//
//            const std::pair<Real, Real> nd = ND(tau, b, interp);
//            return alpha*nd.first/nd.second;
//        };
//
//
//        const auto Dd = [=](Real tau, Real b) -> Real {
//            if (tau < squared(QL_EPSILON)) {
//                if (close_enough(b, K)) {
//                    const Real sqTau = std::sqrt(tau);
//                    const Real vol2 = vol*vol;
//                    return  M_1_SQRTPI*M_SQRT_2*(
//                                -(0.5*vol2 + r-q) / (b*vol*vol2*sqTau)
//                                + 1 / (b*vol*sqTau));
//                }
//                else
//                    return 0.0;
//            }
//            else {
//                const Real dp = d(tau, b/K).first;
//
//                return -phi(dp) * dp / (b*vol*vol*tau) +
//                        phi(dp) / (b*vol * std::sqrt(tau));
//            }
//        };
//
//
//        const auto Nd = [=](Real tau, Real b) -> Real {
//            if (tau < squared(QL_EPSILON)) {
//                if (close_enough(b, K)) {
//                    const Real vol2 = vol*vol;
//                    return M_1_SQRTPI*M_SQRT_2 * (-0.5 * vol2 + r - q)
//                            / (b*vol*vol2*std::sqrt(tau));
//                }
//                else
//                    return 0.0;
//            }
//            else {
//                const Real dm = d(tau, b/K).second;
//                return -phi(dm) * dm / (b*vol*vol*tau);
//            }
//        };
//
//        const auto h = [=](Real fv) -> Real {
//            return squared(std::log(fv/xmax));
//        };
//
//        Array y(x.size());
//        y[0] = 0.0;
//        const Size n_newton
//            = iterationScheme_->getNumberOfJacobiNewtonFixedPointSteps();
//        for (Size k=0; k < n_newton; ++k) {
//            for (Size i=1; i < x.size(); ++i) {
//                const Real tau = squared(x[i]);
//                const Real b = B(tau, interp);
//                const Real fv = f(tau, b , interp);
//
//                if (tau < QL_EPSILON)
//                    y[i] = h(fv);
//                else {
//                    const std::pair<Real, Real> nd = ND(tau, b, interp);
//
//                    const Real fd = K*std::exp(-(r-q)*tau)
//                        * (Nd(tau, b)/nd.second
//                              - Dd(tau, b)*nd.first/squared(nd.second));
//
//                    y[i] = h(b - (fv - b)/ (fd-1));
//                }
//            }
//            interp = ext::make_shared<ChebyshevInterpolation>(y);
//        }
//
//        const Size n_fp = iterationScheme_->getNumberOfNaiveFixedPointSteps();
//        for (Size k=0; k < n_fp; ++k) {
//            for (Size i=1; i < x.size(); ++i) {
//                const Real tau = squared(x[i]);
//                y[i] = h(f(tau, B(tau, interp), interp));
//            }
//            interp = ext::make_shared<ChebyshevInterpolation>(y);
//        }
//
//        const detail::QdPlusAddOnValue aov(T, S, K, r, q, vol, xmax, interp);
//        const Real addOn =
//           (*iterationScheme_->getExerciseBoundaryToPriceIntegrator())(
//               aov, 0.0, std::sqrt(T));
//
//        const Real europeanValue = BlackCalculator(
//            Option::Put, K, S*std::exp((r-q)*T),
//            vol*std::sqrt(T), std::exp(-r*T)).value();
//
//        return std::max(europeanValue, 0.0) + std::max(0.0, addOn);
//    }
}




