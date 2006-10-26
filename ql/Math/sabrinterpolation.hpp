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
#include <ql/Optimization/conjugategradient.hpp>
#include <ql/Optimization/simplex.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <ql/Utilities/null.hpp>
#include <ql/Utilities/dataformatters.hpp>
#include <vector>

namespace QuantLib {

    inline Real unsafeSabrVolatility(Rate strike,
                                     Rate forward,
                                     Time expiryTime,
                                     Real alpha,
                                     Real beta,
                                     Real nu,
                                     Real rho) {
        const Real oneMinusBeta = 1.0-beta;
        const Real A = std::pow(forward*strike, oneMinusBeta);
        const Real sqrtA= std::sqrt(A);
        const Real logM = std::log(forward/strike);
        const Real z = (nu/alpha)*sqrtA*logM;
        const Real B = 1.0-2.0*rho*z+z*z;
        const Real C = oneMinusBeta*oneMinusBeta*logM*logM;
        const Real tmp = (std::sqrt(B)+z-rho)/(1.0-rho);
        const Real xx = std::log(tmp);
        const Real D = sqrtA*(1.0+C/24.0+C*C/1920.0);
        const Real d = 1.0 + expiryTime *
            (oneMinusBeta*oneMinusBeta*alpha*alpha/(24.0*A)
                                + 0.25*rho*beta*nu*alpha/sqrtA
                                    +(2.0-3.0*rho*rho)*(nu*nu/24.0));
        const Real multiplier = (xx!=0.0 ? z/xx : 1.0);
        return (alpha/D)*multiplier*d;
    }

    inline Real sabrVolatility(Rate strike,
                               Rate forward,
                               Time expiryTime,
                               Real alpha,
                               Real beta,
                               Real nu,
                               Real rho) {
        QL_REQUIRE(strike>0.0, "strike must be positive: "
                               << io::rate(strike) << " not allowed");
        QL_REQUIRE(forward>0.0, "forward must be positive: "
                                << io::rate(forward) << " not allowed");
        QL_REQUIRE(expiryTime>0.0, "expiry time must be positive: "
                                   << expiryTime << " not allowed");
        QL_REQUIRE(alpha>0.0, "alpha must be positive: "
                              << alpha << " not allowed");
        QL_REQUIRE(beta>=0.0 && beta<=1.0, "beta must be in [0.0, 1.0]: "
                                           << beta << " not allowed");
        QL_REQUIRE(nu>=0.0, "nu must be non negative: "
                            << nu << " not allowed");
        QL_REQUIRE(rho*rho<=1.0, "rho square must be not greater than one: "
                                 << rho << " not allowed");
        return unsafeSabrVolatility(strike, forward, expiryTime,
                                    alpha, beta, nu, rho);
    }

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
                                  bool isAlphaFixed,
                                  bool isBetaFixed,
                                  bool isNuFixed,
                                  bool isRhoFixed)
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

                if (alpha_ != Null<Real>()) {
                    alphaIsFixed_ = isAlphaFixed;
                    QL_REQUIRE(alpha>0.0,
                               "alpha must be positive: " <<
                               alpha << " not allowed");
                } else alpha_ = std::sqrt(0.2);

                if (beta_!= Null<Real>()) {
                    betaIsFixed_ = isBetaFixed;
                    QL_REQUIRE(beta>=0.0 && beta<=1.0,
                               "beta must be in [0.0, 1.0]: " <<
                               beta << " not allowed");
                } else beta_=0.5;

                if (nu_!= Null<Real>()) {
                    nuIsFixed_ = isNuFixed;
                    QL_REQUIRE(nu>=0.0,
                               "nu must be non negative: " <<
                               nu << " not allowed");
                } else nu_ = std::sqrt(0.4);

                if (rho   != Null<Real>()) {
                    rhoIsFixed_ = isRhoFixed;
                    QL_REQUIRE(rho*rho<=1,
                               "rho square must be not greater than one: " <<
                               rho << " not allowed");
                } else rho_ = 0.0;
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
        /*! */
        template <class I1, class I2>
        SABRInterpolation(const I1& xBegin,     /* x = strikes */
                          const I1& xEnd,
                          const I2& yBegin,     /* y = market volatilities */
                          Time t,               /* option expiry */
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

            impl_ = boost::shared_ptr<Interpolation::Impl>(
                        new detail::SABRInterpolationImpl<I1,I2>(
                            xBegin, xEnd, yBegin,
                            t, forward,
                            alpha, beta, nu, rho, 
                            isAlphaFixed,
                            isBetaFixed,
                            isNuFixed,
                            isRhoFixed,
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


    namespace detail {

        template <class I1, class I2>
        class SABRInterpolationImpl
            : public Interpolation::templateImpl<I1,I2>,
              public SABRCoefficientHolder 
        {
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
            boost::shared_ptr<OptimizationMethod> method_;
            boost::shared_ptr<Transformation> tranformation_;

            std::vector<Real> weights_;
            Real weightsSum_;

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
			  method_(method), weights_(xEnd-xBegin, 1.0), weightsSum_(xEnd-xBegin)
            {
                if (vegaWeighted) {
                    std::vector<Real>::const_iterator i = this->xBegin_;
                    std::vector<Real>::const_iterator j = this->yBegin_;
                    std::vector<Real>::iterator k = weights_.begin();
                    weightsSum_ = 0.0;
                    for ( ; i!=this->xEnd_; ++i, ++j, ++k) {
                        Real stdDev = std::sqrt((*j)*(*j)*t);
                        *k = blackStdDevDerivative(*i, forward, stdDev);
                        weightsSum_ += *k;
                    }
                    k = weights_.begin();
                    for ( ; k!=weights_.end(); ++k)
                        *k /= weightsSum_;
                }
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
                } else if (betaIsFixed_ && !alphaIsFixed_ && !nuIsFixed_ && !rhoIsFixed_) {
                    tranformation_ = boost::shared_ptr<Transformation>(new
                        SabrParametersTransformationWithFixedBeta);
                    NoConstraint constraint;
                    SABRErrorWithFixedBeta costFunction(this);

                    if (!method_) {
                        boost::shared_ptr<LineSearch> lineSearch(new
                            ArmijoLineSearch(1e-12, 0.15, 0.55));
                        method_ = boost::shared_ptr<OptimizationMethod>(new
                            ConjugateGradient(lineSearch));
                        //method_ = boost::shared_ptr<OptimizationMethod>(
                        //    new Simplex(10, .00000001));

                        method_->setEndCriteria(EndCriteria(30000, 1e-8));

                        Array guess(3);
                        guess[0] = alpha_;  
                        guess[1] = nu_; 
                        guess[2] = rho_;
                      
                        guess = tranformation_->inverse(guess);
                        method_->setInitialValue(guess);
                    }

                    Problem problem(costFunction, constraint, *method_);
                    problem.minimize();
				    Array result = problem.minimumValue();

                    Array y = tranformation_->direct(result);
                    alpha_ = y[0];
                    nu_    = y[1];
                    rho_   = y[2]; 

                } else if (!betaIsFixed_ && !alphaIsFixed_ && !nuIsFixed_ && !rhoIsFixed_) {

                    tranformation_ = boost::shared_ptr<Transformation>(new
                        SabrParametersTransformation);
                    NoConstraint constraint;
                    SABRError costFunction(this);

                    if (!method_) {
                        boost::shared_ptr<LineSearch> lineSearch(new
                            ArmijoLineSearch(1e-12, 0.15, 0.55));
                        method_ = boost::shared_ptr<OptimizationMethod>(new
                            ConjugateGradient(lineSearch));
                        //method_ = boost::shared_ptr<OptimizationMethod>(
                        //    new Simplex(10, .00000001));

                        method_->setEndCriteria(EndCriteria(30000, 1e-8));

                        Array guess(4);
                        guess[0] = alpha_;  
                        guess[1] = beta_; 
                        guess[2] = nu_;
                        guess[3] = rho_;

                        guess = tranformation_->inverse(guess);
                        method_->setInitialValue(guess);
                    }

                    Problem problem(costFunction, constraint, *method_);
                    problem.minimize();
				    Array result = problem.minimumValue();

                    Array y = tranformation_->direct(result);
                    alpha_ = y[0];
                    beta_  = y[1];
                    nu_    = y[2];
                    rho_   = y[3]; 
                } else {
                    QL_REQUIRE(false, "Selected Sabr calibration not implemented");
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
            Real primitive(Real x) const {
                QL_FAIL("SABR primitive not implemented");
            }
            Real derivative(Real x) const {
                QL_FAIL("SABR derivative not implemented");
            }
            Real secondDerivative(Real x) const {
                QL_FAIL("SABR secondDerivative not implemented");
            }

            Real interpolationSquaredError() const {
                Real error, totalError = 0.0;
                std::vector<Real>::const_iterator i = this->xBegin_;
                std::vector<Real>::const_iterator j = this->yBegin_;
                std::vector<Real>::const_iterator k = weights_.begin();
                for (; i != this->xEnd_; ++i, ++j, ++k) {
                    error = (value(*i) - *j) * (*k);
                    totalError += error*error;
                }
                return totalError;
            }

            Real interpolationError() const {
                Real squaredError = interpolationSquaredError();
                return std::sqrt(squaredError);
            }

            Real interpolationMaxError() const {
                Real error, maxError = QL_MIN_REAL;
                std::vector<Real>::const_iterator i = this->xBegin_;
                std::vector<Real>::const_iterator j = this->yBegin_;
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
