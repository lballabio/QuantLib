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

#include <iostream>

namespace QuantLib {
    /* Gauss-Legendre (l,m,n)-p Scheme
         l: order of Gauss-Legendre integration within every fixed point iterations
            step.
         m: fixed point iteration steps, first step is a partial Jacobi-Newton,
            the rest are naive Richardson fixed point iterations
         n: number of Chebyshev nodes to interpolate the exercise boundary
         p: order of Gauss-Legendre integration in final conversion of the
            exercise boundary into option prices
    */
    QdFpLegendreIterationScheme::QdFpLegendreIterationScheme(
        Size l, Size m, Size n, Size p):
        m_(m), n_(n),
        fpIntegrator_(ext::make_shared<GaussLegendreIntegrator>(l)),
        exerciseBoundaryIntegrator_(
            ext::make_shared<GaussLegendreIntegrator>(p)) {

        QL_REQUIRE(m_ > 0, "at least one fixed point iteration step is needed");
        QL_REQUIRE(n_ > 0, "at least one interpolation points is needed");
    }

    Size QdFpLegendreIterationScheme::getNumberOfChebyshevInterpolationNodes()
        const {
        return n_;
    }
    Size QdFpLegendreIterationScheme::getNumberOfNaiveFixedPointSteps() const {
        return m_-1;
    }
    Size QdFpLegendreIterationScheme::getNumberOfJacobiNewtonFixedPointSteps()
        const {
        return Size(1);
    }

    ext::shared_ptr<Integrator>
        QdFpLegendreIterationScheme::getFixedPointIntegrator() const {
        return fpIntegrator_;
    }
    ext::shared_ptr<Integrator>
        QdFpLegendreIterationScheme::getExerciseBoundaryToPriceIntegrator()
        const {
        return exerciseBoundaryIntegrator_;
    }

    ext::shared_ptr<QdFpIterationScheme>
        QdFpIterationSchemeStdFactory::fastScheme_
            = ext::make_shared<QdFpLegendreIterationScheme>(12, 1, 12, 24);

    ext::shared_ptr<QdFpIterationScheme>
    QdFpIterationSchemeStdFactory::fastScheme() {
        return QdFpIterationSchemeStdFactory::fastScheme_;
    }

    QdFpAmericanEngine::QdFpAmericanEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess,
        ext::shared_ptr<QdFpIterationScheme> iterationScheme)
    : detail::QdPutCallParityEngine(std::move(bsProcess)),
      iterationScheme_(std::move(iterationScheme)) {
    }

    Real QdFpAmericanEngine::calculatePut(
        Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const {

        const Real xmax =  QdPlusAmericanEngine::xMax(K, r, q);
        const Size n = iterationScheme_->getNumberOfChebyshevInterpolationNodes();

        ext::shared_ptr<ChebyshevInterpolation> interp =
            QdPlusAmericanEngine(process_, n+1, QdPlusAmericanEngine::Halley, 1e-2)
                .getPutExerciseBoundary(S, K, r, q, vol, T);

        const Array z = interp->nodes();
        const Array x = 0.5*std::sqrt(T)*(1.0+z);

        const auto B = [xmax, T](Real tau, const ext::shared_ptr<Interpolation>& interp)
            -> Real {
            const Real z = 2*std::sqrt(std::abs(tau)/T)-1;
            return xmax*std::exp(-std::sqrt(std::max(Real(0), (*interp)(z, true))));
        };

        const NormalDistribution phi;
        const CumulativeNormalDistribution Phi;

        const auto d = [=](Time t, Real z) -> std::pair<Real, Real>  {
            const Real v = vol * std::sqrt(t);
            const Real m = (std::log(z) + (r-q)*t)/v + 0.5*v;

            return std::make_pair(m, m-v);
        };

        const auto K12 = [=, this](Real tau, const ext::shared_ptr<Interpolation>& interp)
            -> Real {
            const Real stv = std::sqrt(tau)/vol;

            return (*iterationScheme_->getFixedPointIntegrator())(
                [=](Real y) -> Real {

                    const Real m = 0.25*tau*squared(1+y);
                    const Real dp =
                        d(0.25*tau*squared(1+y), B(tau, interp)/B(tau-m, interp)).first;

                    return std::exp(q*tau - q*m)
                        *(0.5*tau*(y+1)*Phi(dp) + stv*phi(dp));
                }, -1, 1);
        };

        const auto K3 = [=, this](Real tau, const ext::shared_ptr<Interpolation>& interp)
            -> Real {
            const Real stv = std::sqrt(tau)/vol;

            return (*iterationScheme_->getFixedPointIntegrator())(
                [=](Real y) -> Real {

                    const Real m = 0.25*tau*squared(1+y);
                    return stv*std::exp(r*tau-r*m)*phi(
                        d(0.25*tau*squared(1+y), B(tau, interp)/B(tau-m, interp)).second);
                }, -1, 1);
        };

        const auto ND = [=](
            Real tau, Real b, const ext::shared_ptr<Interpolation>& interp)
            -> std::pair<Real, Real> {

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
                const std::pair<Real, Real> dpm = d(tau, b/K);
                N = phi(dpm.second)/v + r*K3(tau, interp);
                D = phi(dpm.first)/v + Phi(dpm.first) + q*K12(tau, interp);
            }

            return std::make_pair(N, D);
        };

        const auto f = [=](
            Real tau, Real b, const ext::shared_ptr<Interpolation>& interp) -> Real {
            const Real alpha = K*std::exp(-(r-q)*tau);

            if (tau < squared(QL_EPSILON)) {
                if (close_enough(b, K))
                    return alpha;
                else if (b > K)
                    return 0.0;
                else {
                    if (close_enough(q, Real(0)))
                        return alpha*((q < 0)? -1.0 : 1.0)/QL_EPSILON;
                    else
                        return alpha*r/q;
                }
            }

            const std::pair<Real, Real> nd = ND(tau, b, interp);
            return alpha*nd.first/nd.second;
        };


        const auto Dd = [=](
            Real tau, Real b, const ext::shared_ptr<Interpolation>& interp) -> Real {
            if (tau < squared(QL_EPSILON)) {
                if (close_enough(b, K)) {
                    const Real sqTau = std::sqrt(tau);
                    const Real vol2 = vol*vol;
                    return  M_1_SQRTPI*M_SQRT_2*(
                                -(0.5*vol2 + r-q) / (b*vol*vol2*sqTau)
                                + 1 / (b*vol*sqTau));
                }
                else
                    return 0.0;
            }
            else {
                const Real dp = d(tau, b/K).first;

                return -phi(dp) * dp / (b*vol*vol*tau) +
                        phi(dp) / (b*vol * std::sqrt(tau));
            }
        };


        const auto Nd = [=](
            Real tau, Real b, const ext::shared_ptr<Interpolation>& interp) -> Real {
            if (tau < squared(QL_EPSILON)) {
                if (close_enough(b, K)) {
                    const Real vol2 = vol*vol;
                    return M_1_SQRTPI*M_SQRT_2 * (-0.5 * vol2 + r - q)
                            / (b*vol*vol2*std::sqrt(tau));
                }
                else
                    return 0.0;
            }
            else {
                const Real dm = d(tau, b/K).second;
                return -phi(dm) * dm / (b*vol*vol*tau);
            }
        };

        const auto h = [=](Real fv) -> Real {
            return squared(std::log(fv/xmax));
        };

        Array y(x.size());
        const Size n_newton = iterationScheme_->getNumberOfJacobiNewtonFixedPointSteps();
        for (Size k=0; k < n_newton; ++k) {
            for (Size i=0; i < x.size(); ++i) {
                const Real tau = squared(x[i]);
                const Real b = B(tau, interp);
                const Real fv = f(tau, b , interp);

                if (tau < QL_EPSILON)
                    y[i] = h(fv);
                else {
                    const std::pair<Real, Real> nd = ND(tau, b, interp);

                    const Real fd = K*std::exp(-(r-q)*tau)
                        * (Nd(tau, b, interp)/nd.second
                              - Dd(tau, b, interp)*nd.first/squared(nd.second));

                    y[i] = h(b - (fv - b)/ (fd-1));
                }
            }
            interp = ext::make_shared<ChebyshevInterpolation>(y);
        }

        const Size n_fp = iterationScheme_->getNumberOfNaiveFixedPointSteps();
        for (Size k=0; k < n_fp; ++k) {
            for (Size i=0; i < x.size(); ++i) {
                const Real tau = squared(x[i]);
                y[i] = h(f(tau, B(tau, interp), interp));
            }
            interp = ext::make_shared<ChebyshevInterpolation>(y);
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
}




