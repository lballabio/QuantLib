/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2008 Klaus Spanderen
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/math/integrals/discreteintegrals.hpp>
#include <ql/math/integrals/expsinhintegral.hpp>
#include <ql/math/integrals/fourierintegration.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/integrals/kronrodintegral.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/math/integrals/trapezoidintegral.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/utilities/null.hpp>
#include <cmath>
#include <limits>
#include <utility>

namespace QuantLib {

    namespace {

        class integrand1 {
          private:
            const Real c_inf_;
            const std::function<Real(Real)> f_;
          public:
            integrand1(Real c_inf, std::function<Real(Real)> f) : c_inf_(c_inf), f_(std::move(f)) {}
            Real operator()(Real x) const {
                if ((1.0-x)*c_inf_ > QL_EPSILON)
                    return f_(-std::log(0.5-0.5*x)/c_inf_)/((1.0-x)*c_inf_);
                else
                    return 0.0;
            }
        };

        class integrand2 {
          private:
            const Real c_inf_;
            const std::function<Real(Real)> f_;
          public:
            integrand2(Real c_inf, std::function<Real(Real)> f) : c_inf_(c_inf), f_(std::move(f)) {}
            Real operator()(Real x) const {
                if (x*c_inf_ > QL_EPSILON) {
                    return f_(-std::log(x)/c_inf_)/(x*c_inf_);
                } else {
                    return 0.0;
                }
            }
        };

        class integrand3 {
          private:
            const integrand2 int_;
          public:
            integrand3(Real c_inf, const std::function<Real(Real)>& f)
            : int_(c_inf, f) {}

            Real operator()(Real x) const { return int_(1.0-x); }
        };

        class u_Max {
          public:
            u_Max(Real c_inf, Real epsilon) : c_inf_(c_inf), logEpsilon_(std::log(epsilon)) {}

            Real operator()(Real u) const {
                ++evaluations_;
                return c_inf_*u + std::log(u) + logEpsilon_;
            }

            Size evaluations() const { return evaluations_; }

          private:
            const Real c_inf_, logEpsilon_;
            mutable Size evaluations_ = 0;
        };


        class uHat_Max {
          public:
            uHat_Max(Real v0T2, Real epsilon) : v0T2_(v0T2), logEpsilon_(std::log(epsilon)) {}

            Real operator()(Real u) const {
                ++evaluations_;
                return v0T2_*u*u + std::log(u) + logEpsilon_;
            }

            Size evaluations() const { return evaluations_; }

          private:
            const Real v0T2_, logEpsilon_;
            mutable Size evaluations_ = 0;
        };
    }


    FourierIntegration::FourierIntegration(Algorithm intAlgo,
                                           ext::shared_ptr<Integrator> integrator)
    : intAlgo_(intAlgo), integrator_(std::move(integrator)) {}

    FourierIntegration::FourierIntegration(
        Algorithm intAlgo, ext::shared_ptr<GaussianQuadrature> gaussianQuadrature)
    : intAlgo_(intAlgo), gaussianQuadrature_(std::move(gaussianQuadrature)) {}

    FourierIntegration FourierIntegration::gaussLobatto(
       Real relTolerance, Real absTolerance, Size maxEvaluations, bool useConvergenceEstimate) {
       return FourierIntegration(GaussLobatto,
                           ext::make_shared<GaussLobattoIntegral>(maxEvaluations,
                                                        absTolerance,
                                                        relTolerance,
                                                        useConvergenceEstimate));
    }

    FourierIntegration FourierIntegration::gaussKronrod(Real absTolerance,
                                                        Size maxEvaluations) {
        return FourierIntegration(GaussKronrod,
                           ext::make_shared<GaussKronrodAdaptive>(absTolerance,
                                                        maxEvaluations));
    }

    FourierIntegration FourierIntegration::simpson(Real absTolerance,
                                                   Size maxEvaluations) {
        return FourierIntegration(Simpson,
                           ext::make_shared<SimpsonIntegral>(absTolerance,
                                                   maxEvaluations));
    }

    FourierIntegration FourierIntegration::trapezoid(Real absTolerance,
                                                     Size maxEvaluations) {
        return FourierIntegration(Trapezoid,
                           ext::make_shared<TrapezoidIntegral<Default>>(absTolerance,
                                                             maxEvaluations));
    }

    FourierIntegration FourierIntegration::gaussLaguerre(Size intOrder) {
        QL_REQUIRE(intOrder <= 192, "maximum integraton order (192) exceeded");
        return FourierIntegration(GaussLaguerre,
                           ext::make_shared<GaussLaguerreIntegration>(intOrder));
    }

    FourierIntegration FourierIntegration::gaussLegendre(Size intOrder) {
        return FourierIntegration(GaussLegendre,
                           ext::make_shared<GaussLegendreIntegration>(intOrder));
    }

    FourierIntegration FourierIntegration::gaussChebyshev(Size intOrder) {
        return FourierIntegration(GaussChebyshev,
                           ext::make_shared<GaussChebyshevIntegration>(intOrder));
    }

    FourierIntegration FourierIntegration::gaussChebyshev2nd(Size intOrder) {
        return FourierIntegration(GaussChebyshev2nd,
                           ext::make_shared<GaussChebyshev2ndIntegration>(intOrder));
    }

    FourierIntegration FourierIntegration::discreteSimpson(Size evaluations) {
        return FourierIntegration(
            DiscreteSimpson, ext::make_shared<DiscreteSimpsonIntegrator>(evaluations));
    }

    FourierIntegration FourierIntegration::discreteTrapezoid(Size evaluations) {
        return FourierIntegration(
            DiscreteTrapezoid, ext::make_shared<DiscreteTrapezoidIntegrator>(evaluations));
    }

    FourierIntegration FourierIntegration::expSinh(Real relTolerance) {
        return FourierIntegration(
            ExpSinh, ext::make_shared<ExpSinhIntegral>(relTolerance));
    }

    Size FourierIntegration::numberOfEvaluations() const {
        if (integrator_ != nullptr) {
            return integrator_->numberOfEvaluations();
        } else if (gaussianQuadrature_ != nullptr) {
            return gaussianQuadrature_->order();
        } else {
            QL_FAIL("neither Integrator nor GaussianQuadrature given");
        }
    }

    bool FourierIntegration::isAdaptiveIntegration() const {
        return intAlgo_ == GaussLobatto
            || intAlgo_ == GaussKronrod
            || intAlgo_ == Simpson
            || intAlgo_ == Trapezoid
            || intAlgo_ == ExpSinh;
    }

    Real FourierIntegration::calculate(
        Real c_inf,
        const std::function<Real(Real)>& f,
        const std::function<Real()>& maxBound,
        const Real scaling) const {

        Real retVal;

        switch(intAlgo_) {
          case GaussLaguerre:
            retVal = (*gaussianQuadrature_)(f);
            break;
          case GaussLegendre:
          case GaussChebyshev:
          case GaussChebyshev2nd:
            retVal = (*gaussianQuadrature_)(integrand1(c_inf, f));
            break;
          case ExpSinh:
            retVal = scaling*(*integrator_)(
                [scaling, f](Real x) -> Real { return f(scaling*x);},
                0.0, std::numeric_limits<Real>::max());
            break;
          case Simpson:
          case Trapezoid:
          case GaussLobatto:
          case GaussKronrod:
              if (maxBound && maxBound() != Null<Real>())
                  retVal = (*integrator_)(f, 0.0, maxBound());
              else
                  retVal = (*integrator_)(integrand2(c_inf, f), 0.0, 1.0);
              break;
          case DiscreteTrapezoid:
          case DiscreteSimpson:
              if (maxBound && maxBound() != Null<Real>())
                  retVal = (*integrator_)(f, 0.0, maxBound());
              else
                  retVal = (*integrator_)(integrand3(c_inf, f), 0.0, 1.0);
              break;
          default:
            QL_FAIL("unknwon integration algorithm");
        }

        return retVal;
     }

    Real FourierIntegration::calculate(
        Real c_inf,
        const std::function<Real(Real)>& f,
        Real maxBound) const {

        return FourierIntegration::calculate(
            c_inf, f, [=](){ return maxBound; });
    }

    Real FourierIntegration::andersenPiterbargIntegrationLimit(
        Real c_inf, Real epsilon, Real v0, Real t) {

        const Real uMaxGuess = -std::log(epsilon)/c_inf;
        const Real uMaxStep = 0.1*uMaxGuess;

        const Real uMax = Brent().solve(u_Max(c_inf, epsilon),
            QL_EPSILON*uMaxGuess, uMaxGuess, uMaxStep);

        try {
            const Real uHatMaxGuess = std::sqrt(-std::log(epsilon)/(0.5*v0*t));
            const Real uHatMax = Brent().solve(uHat_Max(0.5*v0*t, epsilon),
                QL_EPSILON*uHatMaxGuess, uHatMaxGuess, 0.001*uHatMaxGuess);

            return std::max(uMax, uHatMax);
        }
        catch (const Error&) {
            return uMax;
        }
    }
}
