/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file sabrinterpolation.hpp
    \brief SABR interpolation interpolation between discrete points
*/

#ifndef quantlib_sabr_interpolation_hpp
#define quantlib_sabr_interpolation_hpp

#include <ql/Math/interpolation.hpp>
#include <ql/Optimization/method.hpp>
#include <ql/Optimization/problem.hpp>
#include <ql/Optimization/armijo.hpp>
#include <ql/Optimization/conjugategradient.hpp>
#include <ql/Optimization/simplex.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <ql/Utilities/null.hpp>
#include <ql/Utilities/dataformatters.hpp>
#include <ql/Volatilities/sabr.hpp>
#include <vector>

namespace QuantLib {

    namespace detail {

        template <class I1, class I2> class SABRInterpolationImpl;

        class SABRCoefficientHolder {
          public:
            SABRCoefficientHolder(Time t,
                                  Real forward,
                                  Real alpha,
                                  Real beta,
                                  Real nu,
                                  Real rho,
                                  bool alphaIsFixed,
                                  bool betaIsFixed,
                                  bool nuIsFixed,
                                  bool rhoIsFixed)
            : t_(t), forward_(forward),
              alpha_(alpha), beta_(beta), nu_(nu), rho_(rho),
              alphaIsFixed_(false),
              betaIsFixed_(false),
              nuIsFixed_(false),
              rhoIsFixed_(false),
              error_(Null<Real>()),
              maxError_(Null<Real>()),
              SABREndCriteria_(EndCriteria::none)
            {
                QL_REQUIRE(forward>0.0, "forward must be positive: "
                    << io::rate(forward) << " not allowed");
                QL_REQUIRE(t>0.0, "expiry time must be positive: "
                                  << t << " not allowed");

                if (alpha_ != Null<Real>())
                    alphaIsFixed_ = alphaIsFixed;
                else alpha_ = std::sqrt(0.2);

                if (beta_ != Null<Real>())
                    betaIsFixed_ = betaIsFixed;
                else beta_=0.5;

                if (nu_ != Null<Real>())
                    nuIsFixed_ = nuIsFixed;
                else nu_ = std::sqrt(0.4);

                if (rho_ != Null<Real>())
                    rhoIsFixed_ = rhoIsFixed;
                else rho_ = 0.0;

                validateSabrParameters(alpha_, beta_, nu_, rho_);
            }
            virtual ~SABRCoefficientHolder() {}

            /*! Option expiry */
            Real t_;
            /*! */
            Real forward_;
            /*! Sabr parameters */
            Real alpha_, beta_, nu_, rho_;
            bool alphaIsFixed_, betaIsFixed_, nuIsFixed_, rhoIsFixed_;
            Real error_, maxError_;
            EndCriteria::Type SABREndCriteria_;
        };

    }

    //! %SABR smile interpolation between discrete volatility points.
    class SABRInterpolation : public Interpolation {
      public:
        template <class I1, class I2>
        SABRInterpolation(const I1& xBegin,  // x = strikes
                          const I1& xEnd,
                          const I2& yBegin,  // y = volatilities
                          Time t,            // option expiry
                          Real forward,
                          Real alpha,
                          Real beta,
                          Real nu,
                          Real rho,
                          bool isAlphaFixed,
                          bool isBetaFixed,
                          bool isNuFixed,
                          bool isRhoFixed,
                          bool vegaWeighted = false,
                          const boost::shared_ptr<OptimizationMethod>& method
                                  = boost::shared_ptr<OptimizationMethod>()) {

            impl_ = boost::shared_ptr<Interpolation::Impl>(new
                detail::SABRInterpolationImpl<I1,I2>(xBegin, xEnd, yBegin,
                                                     t, forward,
                                                     alpha, beta, nu, rho,
                                                     isAlphaFixed, isBetaFixed,
                                                     isNuFixed, isRhoFixed,
                                                     vegaWeighted,
                                                     method));
            coeffs_ =
                boost::dynamic_pointer_cast<detail::SABRCoefficientHolder>(
                                                                       impl_);
        }

        Real expiry()  const { return coeffs_->t_; }
        Real forward() const { return coeffs_->forward_; }
        Real alpha()   const { return coeffs_->alpha_; }
        Real beta()    const { return coeffs_->beta_; }
        Real nu()      const { return coeffs_->nu_; }
        Real rho()     const { return coeffs_->rho_; }

        Real interpolationError() const { return coeffs_->error_; }
        Real interpolationMaxError() const { return coeffs_->maxError_; }
        //const std::vector<Real>& interpolationWeights() const {
        //    return impl_->weights_; }
        EndCriteria::Type endCriteria(){ return coeffs_->SABREndCriteria_; }

      private:
        boost::shared_ptr<detail::SABRCoefficientHolder> coeffs_;
    };


    //! %SABR interpolation factory
    class SABR {
    public:
        SABR(Time t, 
             Real forward, 
             Real alpha, 
             Real beta,
             Real nu,
             Real rho,
             bool isAlphaFixed,
             bool isBetaFixed,
             bool isNuFixed,
             bool isRhoFixed,
             bool vegaWeighted = false,
             const boost::shared_ptr<OptimizationMethod> method
                = boost::shared_ptr<OptimizationMethod>()): 
                t_(t), forward_(forward), alpha_(alpha), beta_(beta),nu_(nu),
                rho_(rho), isAlphaFixed_(isAlphaFixed),
                isBetaFixed_(isBetaFixed), isNuFixed_(isNuFixed),
                isRhoFixed_(isRhoFixed), vegaWeighted_(vegaWeighted),
                method_(method){}
                SABR(){};
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return SABRInterpolation(xBegin, xEnd, yBegin, t_,  forward_,
                alpha_, beta_, nu_, rho_, isAlphaFixed_, isBetaFixed_,
                isNuFixed_, isRhoFixed_, vegaWeighted_, 
                method_);
    }

    private:
        Time t_;
        Real forward_;
        Real alpha_;
        Real beta_;
        Real nu_;
        Real rho_;
        bool isAlphaFixed_;
        bool isBetaFixed_;
        bool isNuFixed_;
        bool isRhoFixed_;
        bool vegaWeighted_;
        const boost::shared_ptr<OptimizationMethod> method_;
    };



    namespace detail {

        template <class I1, class I2>
        class SABRInterpolationImpl : public Interpolation::templateImpl<I1,I2>,
                                      public SABRCoefficientHolder {
          private:

            // function to minimize
            class SABRError;
            friend class SABRError;
            class SABRError : public CostFunction {
              public:
                SABRError(SABRInterpolationImpl* sabr)
                : sabr_(sabr) {}
                Real value(const Array& x) const {

                    // INVERSE TRANSFORMATION
                    if (!sabr_->alphaIsFixed_) sabr_->alpha_ = std::max(x[0]*x[0], QL_EPSILON);
                    if (!sabr_->betaIsFixed_)  sabr_->beta_  = std::exp(-(x[1]*x[1]));
                    if (!sabr_->nuIsFixed_)    sabr_->nu_    = x[2]*x[2];
                    if (!sabr_->rhoIsFixed_)   sabr_->rho_   = std::max(std::min(2.0/M_PI*std::atan(x[3]), 1.0-QL_EPSILON), -1.0+QL_EPSILON);

                    return sabr_->interpolationSquaredError();
                }
              private:
                SABRInterpolationImpl* sabr_;
            };

            // optimization method used for fitting
            boost::shared_ptr<OptimizationMethod> method_;
            std::vector<Real> weights_;
          public:
            SABRInterpolationImpl(
                const I1& xBegin, const I1& xEnd,
                const I2& yBegin,
                Time t, Real forward,
                Real alpha, Real beta, Real nu, Real rho,
                bool isAlphaFixed,
                bool isBetaFixed,
                bool isNuFixed,
                bool isRhoFixed,
                bool vegaWeighted,
                const boost::shared_ptr<OptimizationMethod>& method)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              SABRCoefficientHolder(t, forward, alpha, beta, nu, rho,
              isAlphaFixed, isBetaFixed, isNuFixed, isRhoFixed),
              method_(method), weights_(xEnd-xBegin, 1.0) {
                Real weightsSum = this->xEnd_-this->xBegin_;
                if (vegaWeighted) {
                    std::vector<Real>::const_iterator x = this->xBegin_;
                    std::vector<Real>::const_iterator y = this->yBegin_;
                    std::vector<Real>::iterator w = weights_.begin();
                    weightsSum = 0.0;
                    for ( ; x!=this->xEnd_; ++x, ++y, ++w) {
                        Real stdDev = std::sqrt((*y)*(*y)*t);
                        *w = blackStdDevDerivative(*x, forward, stdDev);
                        weightsSum += *w;
                    }
                }
                // weight normalization
                std::vector<Real>::iterator w = weights_.begin();
                for ( ; w!=weights_.end(); ++w)
                    *w /= weightsSum;

                calculate();
            }

            void calculate()
            {
                // there is nothing to optimize
                if (alphaIsFixed_ && betaIsFixed_ && nuIsFixed_ && rhoIsFixed_) {
                    error_ = interpolationError();
                    maxError_ = interpolationMaxError();
                    SABREndCriteria_ = EndCriteria::none;
                    return;
                } else {
                    NoConstraint constraint;
                    SABRError costFunction(this);

                    if (!method_) {
                        // DIRECT TRANSFORMATION
                        Array guess(4);
                        guess[0] = std::sqrt(alpha_);
                        guess[1] = std::sqrt(-std::log(std::max(beta_, QL_EPSILON)));
                        guess[2] = std::sqrt(nu_);
                        guess[3] = std::tan(M_PI/2.0*rho_);

                        EndCriteria endCriteria(60000, 1e-8);

                        //boost::shared_ptr<LineSearch> lineSearch(new
                        //    ArmijoLineSearch(1e-12, 0.15, 0.55));

                        //method_ = boost::shared_ptr<OptimizationMethod>(new
                        //    ConjugateGradient(guess, endCriteria, lineSearch));

                        method_ = boost::shared_ptr<OptimizationMethod>(new
                            Simplex(0.01, guess, endCriteria));
                    } else { // transform the guess
                        // DIRECT TRANSFORMATION
                        Array guess(4);
                        const Array& sabrParameters = method_->x();
                        guess[0] = std::sqrt(sabrParameters[0]);
                        guess[1] = std::sqrt(-std::log(std::max(sabrParameters[1], QL_EPSILON)));
                        guess[2] = std::sqrt(sabrParameters[2]);
                        guess[3] = std::tan(M_PI/2.0*sabrParameters[3]);
                        method_->setInitialValue(guess);
                    }

                    Problem problem(costFunction, constraint, *method_);
                    problem.minimize();
                    const Array& x = problem.minimumValue();

                    // INVERSE TRANSFORMATION
                    if (!alphaIsFixed_) alpha_ = std::max(x[0]*x[0], QL_EPSILON);
                    if (!betaIsFixed_)  beta_  = std::exp(-(x[1]*x[1]));
                    if (!nuIsFixed_)    nu_    = x[2]*x[2];
                    if (!rhoIsFixed_)   rho_   = std::max(std::min(2.0/M_PI*std::atan(x[3]), 1.0-QL_EPSILON), -1.0+QL_EPSILON);
                }

                SABREndCriteria_ = endCriteria();
                error_ = interpolationError();
                maxError_ = interpolationMaxError();
            }

            Real value(Real x) const {
                QL_REQUIRE(x>0.0, "strike must be positive: " <<
                                  io::rate(x) << " not allowed");
                return sabrVolatility(x, forward_, t_,
                                      alpha_, beta_, nu_, rho_);
            }
            Real primitive(Real) const {
                QL_FAIL("SABR primitive not implemented");
            }
            Real derivative(Real) const {
                QL_FAIL("SABR derivative not implemented");
            }
            Real secondDerivative(Real) const {
                QL_FAIL("SABR secondDerivative not implemented");
            }

            Real interpolationSquaredError() const {
                Real error, totalError = 0.0;
                std::vector<Real>::const_iterator x = this->xBegin_;
                std::vector<Real>::const_iterator y = this->yBegin_;
                std::vector<Real>::const_iterator w = weights_.begin();
                for (; x != this->xEnd_; ++x, ++y, ++w) {
                    error = (value(*x) - *y);
                    totalError += error*error * (*w);
                }
                return totalError;
            }

            Real interpolationError() const {
                Size n = this->xEnd_-this->xBegin_;
                Real squaredError = interpolationSquaredError();
                return std::sqrt(n*squaredError/(n-1));
            }

            Real interpolationMaxError() const {
                Real error, maxError = QL_MIN_REAL;
                I1 i = this->xBegin_;
                I2 j = this->yBegin_;
                for (; i != this->xEnd_; ++i, ++j) {
                    error = std::fabs(value(*i) - *j);
                    maxError = std::max(maxError, error);
                }
                return maxError;
            }

            EndCriteria::Type endCriteria() {
                return method_->endCriteria().criteria();
            }
        };

    }

}

#endif
