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
                                  const Real& forward,
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
              SABREndCriteria_(EndCriteria::None)
            {
                QL_REQUIRE(t>0.0, "expiry time must be positive: "
                                  << t << " not allowed");

                if (alpha_ != Null<Real>())
                    alphaIsFixed_ = alphaIsFixed;
                else alpha_ = std::sqrt(0.2);

                if (beta_ != Null<Real>())
                    betaIsFixed_ = betaIsFixed;
                else beta_ = 0.5;

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
            const Real& forward_;
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
                          const Real& forward,
                          Real alpha,
                          Real beta,
                          Real nu,
                          Real rho,
                          bool alphaIsFixed,
                          bool betaIsFixed,
                          bool nuIsFixed,
                          bool rhoIsFixed,
                          bool vegaWeighted = false,
                          const boost::shared_ptr<EndCriteria>& endCriteria
                                  = boost::shared_ptr<EndCriteria>(),
                          const boost::shared_ptr<OptimizationMethod>& method
                                  = boost::shared_ptr<OptimizationMethod>(),
                          bool calculate = true) {

            impl_ = boost::shared_ptr<Interpolation::Impl>(new
                detail::SABRInterpolationImpl<I1,I2>(xBegin, xEnd, yBegin,
                                                     t, forward,
                                                     alpha, beta, nu, rho,
                                                     alphaIsFixed, betaIsFixed,
                                                     nuIsFixed, rhoIsFixed,
                                                     vegaWeighted,
                                                     endCriteria,
                                                     method,
                                                     calculate));
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
        SABR(Time t, Real forward,
             Real alpha, Real beta, Real nu, Real rho,
             bool alphaIsFixed, bool betaIsFixed,
             bool nuIsFixed, bool rhoIsFixed,
             bool vegaWeighted = false,
             const boost::shared_ptr<EndCriteria> endCriteria
                = boost::shared_ptr<EndCriteria>(),
             const boost::shared_ptr<OptimizationMethod> method
                = boost::shared_ptr<OptimizationMethod>())
        : t_(t), forward_(forward),
          alpha_(alpha), beta_(beta), nu_(nu), rho_(rho),
          alphaIsFixed_(alphaIsFixed), betaIsFixed_(betaIsFixed),
          nuIsFixed_(nuIsFixed), rhoIsFixed_(rhoIsFixed),
          vegaWeighted_(vegaWeighted),
          endCriteria_(endCriteria),
          method_(method) {}
        SABR() {};

        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return SABRInterpolation(xBegin, xEnd, yBegin,
                                     t_,  forward_,
                                     alpha_, beta_, nu_, rho_,
                                     alphaIsFixed_, betaIsFixed_,
                                     nuIsFixed_, rhoIsFixed_,
                                     vegaWeighted_, endCriteria_, method_);
        }
      private:
        Time t_;
        Real forward_;
        Real alpha_, beta_, nu_, rho_;
        bool alphaIsFixed_, betaIsFixed_, nuIsFixed_, rhoIsFixed_;
        bool vegaWeighted_;
        const boost::shared_ptr<EndCriteria> endCriteria_;
        const boost::shared_ptr<OptimizationMethod> method_;
    };


    namespace detail {

        template <class I1, class I2>
        class SABRInterpolationImpl : public Interpolation::templateImpl<I1,I2>,
                                      public SABRCoefficientHolder {
          private:
              class Transformation {
              public:
                  virtual ~Transformation() {}
                  virtual Array direct(const Array& x) const = 0;
                  virtual Array inverse(const Array& x) const = 0;
              };

              class SabrParametersTransformation :
                  public Transformation {
                     mutable Array y_;
                     const Real eps1_, eps2_;
             public:

                    SabrParametersTransformation() : y_(Array(4)),
                        eps1_(.0000001),
                        eps2_(.9999) {
                    }

                    Array direct(const Array& x) const {
                        y_[0] = x[0]*x[0] + eps1_;
                        //y_[1] = std::abs(eps2 * std::sin(x[1]));
                        y_[1] = std::exp(-(x[1]*x[1]));
                        y_[2] = x[2]*x[2] + eps1_;
                        y_[3] = eps2_ * std::sin(x[3]);
                        return y_;
                    }
                    Array inverse(const Array& x) const {
                     y_[0] = std::sqrt(x[0] - eps1_);
                     y_[1] = std::sqrt(-std::log(x[1]));
                     y_[2] = std::sqrt(x[2] - eps1_);
                     {
                         //arcsin expansion
                         const Real z = x[3]/eps2_;
                         const Real z3 = z*z*z;
                         const Real z5 = z3*z*z;
                         y_[3] = z+z3/6 +3*z5/40;
                     }
                     return y_;
                    }
             };

             class SabrParametersTransformationWithFixedBeta :
                 public Transformation {
                    mutable Array y_;
                    const Real eps1_, eps2_;
             public:

                 SabrParametersTransformationWithFixedBeta() : y_(Array(3)),
                     eps1_(.0000001),
                     eps2_(.9999) {
                 }
                 Array direct(const Array& x) const {
                     y_[0] = x[0]*x[0] + eps1_;
                     y_[1] = x[1]*x[1] + eps1_;
                     y_[2] = eps2_ * std::sin(x[2]);
                     return y_;
                 }
                Array inverse(const Array& x) const {
                    y_[0] = std::sqrt(x[0] - eps1_);
                    y_[1] = std::sqrt(x[1] - eps1_);
                    {
                        //arcsin expansion
                        const Real z = x[2]/eps2_;
                        const Real z3 = z*z*z;
                        const Real z5 = z3*z*z;
                        y_[2] = z+z3/6 +3*z5/40;
                    }
                    return y_;
                }
            };

            // function to minimize
            class SABRError;
            friend class SABRError;
            class SABRError : public CostFunction {
              public:
                SABRError(SABRInterpolationImpl* sabr)
                : sabr_(sabr) {}
                Real value(const Array& x) const {

                    const Array y = sabr_->tranformation_->direct(x);

                    sabr_->alpha_ = y[0];
                    sabr_->beta_  = y[1];
                    sabr_->nu_    = y[2];
                    sabr_->rho_   = y[3];

                    return sabr_->interpolationSquaredError();
                }
              private:
                SABRInterpolationImpl* sabr_;
            };

            class SABRErrorWithFixedBeta;
            friend class SABRErrorWithFixedBeta;
            class SABRErrorWithFixedBeta : public CostFunction {
              public:
                SABRErrorWithFixedBeta(SABRInterpolationImpl* sabr)
                : sabr_(sabr) {}
                Real value(const Array& x) const {

                    const Array y = sabr_->tranformation_->direct(x);

                    sabr_->alpha_ = y[0];
                    sabr_->nu_    = y[1];
                    sabr_->rho_   = y[2];
                    return sabr_->interpolationSquaredError();
                }
              private:
                SABRInterpolationImpl* sabr_;
            };
            // optimization method used for fitting
            boost::shared_ptr<EndCriteria> endCriteria_;
            boost::shared_ptr<OptimizationMethod> method_;
            boost::shared_ptr<Transformation> tranformation_;
            std::vector<Real> weights_;
            const Real& forward_;
            NoConstraint constraint_;

          public:
            SABRInterpolationImpl(
                const I1& xBegin, const I1& xEnd,
                const I2& yBegin,
                Time t, const Real& forward,
                Real alpha, Real beta, Real nu, Real rho,
                bool alphaIsFixed,
                bool betaIsFixed,
                bool nuIsFixed,
                bool rhoIsFixed,
                bool vegaWeighted,
                const boost::shared_ptr<EndCriteria>& endCriteria,
                const boost::shared_ptr<OptimizationMethod>& method,
                bool compute)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              SABRCoefficientHolder(t, forward, alpha, beta, nu, rho,
                                    alphaIsFixed, betaIsFixed, nuIsFixed,
                                    rhoIsFixed),
              endCriteria_(endCriteria), method_(method), weights_(xEnd-xBegin, 1.0), forward_(forward) {
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

                // if no method is provided we provide one
                if (!method_)
                    method_ = boost::shared_ptr<OptimizationMethod>(new
                        Simplex(0.01));

                if (!endCriteria_) {
                    endCriteria_ = boost::shared_ptr<EndCriteria>(new
                        EndCriteria(60000, 1e-8));
                }

                if (compute)
                    calculate();
            }

            void calculate()
            {

                QL_REQUIRE(forward_>0.0, "forward must be positive: "
                    << io::rate(forward_) << " not allowed");
                
                // there is nothing to optimize
                if (alphaIsFixed_ && betaIsFixed_ && nuIsFixed_ && rhoIsFixed_) {
                    error_ = interpolationError();
                    maxError_ = interpolationMaxError();
				    SABREndCriteria_ = EndCriteria::None;
                    return;
                } else if (betaIsFixed_ && !alphaIsFixed_ && !nuIsFixed_ && !rhoIsFixed_) {
                    tranformation_ = boost::shared_ptr<Transformation>(new
                        SabrParametersTransformationWithFixedBeta);
                    NoConstraint constraint;
                    SABRErrorWithFixedBeta costFunction(this);

                    Array guess(3);
                    guess[0] = alpha_;  
                    guess[1] = nu_; 
                    guess[2] = rho_;
                  
                    guess = tranformation_->inverse(guess);

                    Problem problem(costFunction, constraint, guess);
                    SABREndCriteria_ = method_->minimize(problem, *endCriteria_);
				    Array result = problem.currentValue();

                    Array y = tranformation_->direct(result);
                    alpha_ = y[0];
                    nu_    = y[1];
                    rho_   = y[2]; 

                } else if (!betaIsFixed_ && !alphaIsFixed_ && !nuIsFixed_ && !rhoIsFixed_) {

                    tranformation_ = boost::shared_ptr<Transformation>(new
                        SabrParametersTransformation);
                    NoConstraint constraint;
                    SABRError costFunction(this);

                    Array guess(4); 
                    guess[0] = alpha_;  
                    guess[1] = beta_; 
                    guess[2] = nu_;
                    guess[3] = rho_;

                    guess = tranformation_->inverse(guess);

                    Problem problem(costFunction, constraint, guess);
                    SABREndCriteria_ = method_->minimize(problem, *endCriteria_);
				    Array result = problem.currentValue();

                    Array y = tranformation_->direct(result);
                    alpha_ = y[0];
                    beta_  = y[1];
                    nu_    = y[2];
                    rho_   = y[3]; 
                } else {
                    QL_REQUIRE(false, "Selected Sabr calibration not implemented");
                }
     
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
        };

    }

}

#endif
