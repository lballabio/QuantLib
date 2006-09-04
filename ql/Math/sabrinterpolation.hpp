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
#include <ql/Utilities/null.hpp>
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
                                  Real rho)
            : t_(t), forward_(forward),
              alpha_(alpha), beta_(beta), nu_(nu), rho_(rho),
              alphaIsFixed_(false), betaIsFixed_(false),
              nuIsFixed_(false), rhoIsFixed_(false),
              error_(Null<Real>()), maxError_(Null<Real>()),
              SABREndCriteria_(EndCriteria::none)
            {
                QL_REQUIRE(t>0, "time must be non-negative");
                QL_REQUIRE(forward>0, "forward must be non-negative");
                if (alpha != Null<Real>()) {
                    alphaIsFixed_ = true;
                    QL_REQUIRE(alpha_>0.0, "alpha must be positive");
                }
                if (beta  != Null<Real>()) {
                    betaIsFixed_  = true;
                    QL_REQUIRE(beta_>=0.0 && beta_<=1.0,
                               "beta must be in [0.0, 1.0]");
                }
                if (nu    != Null<Real>()) {
                    nuIsFixed_    = true;
                    QL_REQUIRE(nu_>=0.0, "nu must be non negative");
                }
                if (rho   != Null<Real>()) {
                    rhoIsFixed_   = true;
                    QL_REQUIRE(rho_*rho_<1, "rho square must be less than 1");
                }
            }
            virtual ~SABRCoefficientHolder() {}
            Real t_, forward_, alpha_, beta_, nu_, rho_;
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
        SABRInterpolation(const I1& xBegin,
                          const I1& xEnd,
                          const I2& yBegin,
                          Time t,
                          Real forward,
                          Real alpha,
                          Real beta,
                          Real nu,
                          Real rho,
                          const boost::shared_ptr<OptimizationMethod>& method
                                  = boost::shared_ptr<OptimizationMethod>()) {

            impl_ = boost::shared_ptr<Interpolation::Impl>(
                        new detail::SABRInterpolationImpl<I1,I2>(
                            xBegin, xEnd, yBegin,
                            t, forward,
                            alpha, beta, nu, rho,
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
                  virtual Array direct(const Array& x) const = 0;
                  virtual Array inverse(const Array& x) const = 0;
              };

              class SabrParametersTransformation : 
                  public Transformation {
                     mutable Array y_;
                     const Real eps1, eps2;
             public:

                    SabrParametersTransformation() : y_(Array(4)),
                        eps1(.0000001),
                        eps2(.9999) {
                    }

                    Array direct(const Array& x) const {
                        y_[0] = x[0]*x[0] + eps1;
                        //y_[1] = std::abs(eps2 * std::sin(x[1]));
                        y_[1] = std::exp(-x[1]);
                        y_[2] = x[2]*x[2] + eps1;
                        y_[3] = eps2 * std::sin(x[3]);
                        return y_;
                    }
                    Array inverse(const Array& x) const {
                     y_[0] = std::sqrt(x[0] - eps1);
                     y_[1] = -std::log(x[1]);
                     y_[2] = std::sqrt(x[2] - eps1);
                     {
                         //arcsin expansion
                         const Real z = x[3]/eps2;
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
                    const Real eps1, eps2;
             public:

                 SabrParametersTransformationWithFixedBeta() : y_(Array(3)),
                     eps1(.0000001),
                     eps2(.9999) {
                 }
                 Array direct(const Array& x) const {
                     y_[0] = x[0]*x[0] + eps1;
                     y_[1] = x[1]*x[1] + eps1;
                     y_[2] = eps2 * std::sin(x[2]);
                     return y_;
                 }
                Array inverse(const Array& x) const {
                    y_[0] = std::sqrt(x[0] - eps1);
                    y_[1] = std::sqrt(x[1] - eps1);
                    {
                        //arcsin expansion
                        const Real z = x[2]/eps2;
                        const Real z3 = z*z*z;
                        const Real z5 = z3*z*z;
                        y_[2] = z+z3/6 +3*z5/40;
                    }
                    return y_;
                }
            };
             
            // function to minimize
            //class SABRError : public CostFunction {
            //  public:
            //    SABRError(SABRInterpolationImpl* sabr)
            //    : sabr_(sabr) {}
            //    Real value(const Array& x) const {
            //        if (!sabr_->alphaIsFixed_) sabr_->alpha_ = x[0];
            //        if (!sabr_->betaIsFixed_)  sabr_->beta_  = x[1];
            //        if (!sabr_->nuIsFixed_)    sabr_->nu_    = x[2];
            //        if (!sabr_->rhoIsFixed_)   sabr_->rho_   = x[3];

            //        return sabr_->interpolationSquaredNonNormalizedError();
            //    }
            //  private:
            //    SABRInterpolationImpl* sabr_;
            //};
              
            class SABRError : public CostFunction {
              public:
                SABRError(SABRInterpolationImpl* sabr)
                : sabr_(sabr) {}
                Real value(const Array& x) const {

                   Array y = sabr_->tranformation_.direct(x);

                    sabr_->alpha_ = y[0];
                    sabr_->beta_ = y[1];
                    sabr_->nu_   = y[2];
                    sabr_->rho_   = y[3];

                    return sabr_->interpolationSquaredNonNormalizedError();
                }
              private:
                SABRInterpolationImpl* sabr_;
            };

            class SABRErrorWithFixedBeta : public CostFunction {
              public:
                SABRErrorWithFixedBeta(SABRInterpolationImpl* sabr)
                : sabr_(sabr) {}
                Real value(const Array& x) const {

                    Array y = sabr_->tranformation_.direct(x);

                    sabr_->alpha_ = y[0];
                    sabr_->nu_    = y[1];
                    sabr_->rho_   = y[2];
                    return sabr_->interpolationSquaredNonNormalizedError();
                }
              private:
                SABRInterpolationImpl* sabr_;
            };
     
            // optimization constraints
            class SABRConstraint : public Constraint {
              private:
                class Impl : public Constraint::Impl {
                  public:
                    bool test(const Array& params) const {
                        return params[0]>0.0                    // alpha
                            && params[1]>=0.0 && params[0]<=1.0 // beta
                            && params[2]>=0.0                   // nu
                            && params[3]*params[3] < 1.0;       // rho
                    }
                };
              public:
                SABRConstraint()
                : Constraint(boost::shared_ptr<Constraint::Impl>(new Impl)) {}
            };

            // optimization method used for fitting
            boost::shared_ptr<OptimizationMethod> method_;
            SabrParametersTransformationWithFixedBeta tranformation_;

          public:  
            SABRInterpolationImpl(
                const I1& xBegin, const I1& xEnd,
                const I2& yBegin,
                Time t, Real forward,
                Real alpha, Real beta, Real nu, Real rho,
                const boost::shared_ptr<OptimizationMethod>& method)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              SABRCoefficientHolder(t, forward, alpha, beta, nu, rho),
			  method_(method)
            {
                calculate();
            }

            void calculate()
            {
                // there is nothing to optimize
                if (alphaIsFixed_ && betaIsFixed_ && nuIsFixed_ && rhoIsFixed_)
                {
                    error_ = interpolationError();
                    maxError_ = interpolationMaxError(); 
				    SABREndCriteria_ = EndCriteria::none;
                    return;
                } 
                else if (betaIsFixed_ 
                    && !alphaIsFixed_ && !nuIsFixed_ && !rhoIsFixed_) {

                    NoConstraint constraint;
                    SABRErrorWithFixedBeta costFunction(this);

                    if (!method_) {
                        boost::shared_ptr<LineSearch> lineSearch(
                            new ArmijoLineSearch(1e-12, 0.15, 0.55));
                        method_ = boost::shared_ptr<OptimizationMethod>(
                            new ConjugateGradient(lineSearch));
                        //method_ = boost::shared_ptr<OptimizationMethod>(
                        //    new Simplex(10, .00000001));

                        method_->setEndCriteria(EndCriteria(10000, 1e-12));
                        Array guess(3);
                        
                        guess[0] = 0.2;  
                        guess[1] = 0.4; 
                        guess[2] = 0;
                      
                        //guess = f_.inverse(guess);
                        //guess[0] = 0.3;  
                        //guess[1] = 0.;  
                        //guess[2] = 0; 
                        method_->setInitialValue(guess);
                    }

                    Problem problem(costFunction, constraint, *method_);
                    problem.minimize();
				    Array result = problem.minimumValue();

                    Array y = tranformation_.direct(result);
                    alpha_ = y[0];
                    nu_    = y[1];
                    rho_   = y[2]; 

				    SABREndCriteria_ = endCriteria();
                    error_ = interpolationError();
                    maxError_ = interpolationMaxError(); 
                }
                else {

                    NoConstraint constraint;
                    //SABRConstraint constraint;
                    SABRError costFunction(this);

                    if (!method_) {
                        boost::shared_ptr<LineSearch> lineSearch(
                            new ArmijoLineSearch(1e-12, 0.15, 0.55));
                        method_ = boost::shared_ptr<OptimizationMethod>(
                            new ConjugateGradient(lineSearch));
                        method_->setEndCriteria(EndCriteria(100000, 1e-12));
                        Array guess(4);
                        guess[0] = 0.05;  // alpha
                        guess[1] = 0.69;  // beta
                        guess[2] = 0.30;  // nu
                        guess[3] = -0.17; // rho
                        method_->setInitialValue(guess);
                    }

                    Problem problem(costFunction, constraint, *method_);
                    problem.minimize();
				    Array result = problem.minimumValue();

                    Array y = tranformation_.direct(result);
                    alpha_ = y[0];
                    beta_ = y[1];
                    nu_    = y[2];
                    rho_   = y[3]; 
                    //if (!alphaIsFixed_) alpha_ = result[0];
                    //if (!betaIsFixed_)  beta_  = result[1];
                    //if (!nuIsFixed_)    nu_    = result[2];
                    //if (!rhoIsFixed_)   rho_   = result[3];

                    //QL_ENSURE(alpha_>0.0, "alpha must be positive");
                    //QL_ENSURE(beta_>=0.0 && beta_<=1.0,
                    //        "beta must be in [0.0, 1.0]");
                    //QL_ENSURE(nu_>=0.0, "nu must be non negative");
                    //QL_ENSURE(rho_*rho_<1, "rho square must be less than 1");

				    SABREndCriteria_ = endCriteria();
                    error_ = interpolationError();
                    maxError_ = interpolationMaxError(); 
                }
            }

            Real value(Real x) const {
                QL_REQUIRE(x>0.0, 
                    "strike must be positive in Sabr function");
                const Real oneMinusBeta = 1.0-beta_;
                const Real A = std::pow(forward_*x, oneMinusBeta);
                const Real sqrtA= std::sqrt(A);
                const Real logM = std::log(forward_/x);
                const Real z = (nu_/alpha_)*sqrtA*logM;
                const Real B = 1.0-2.0*rho_*z+z*z;
                const Real C = oneMinusBeta*oneMinusBeta*logM*logM;
                const Real tmp = (std::sqrt(B)+z-rho_)/(1.0-rho_);
                const Real xx = std::log(tmp);
                const Real D = sqrtA*(1.0+C/24.0+C*C/1920.0);
                const Real d = 1.0 +
                    t_ * (oneMinusBeta*oneMinusBeta*alpha_*alpha_/(24.0*A)
                          + 0.25*rho_*beta_*nu_*alpha_/sqrtA
                          +(2.0-3.0*rho_*rho_)*(nu_*nu_/24));
                const Real multiplier = (xx!=0.0 ? z/xx : 1.0);
                return (alpha_/D)*multiplier*d;;
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

            Real interpolationSquaredNonNormalizedError() const {
                Real error, totalError = 0.0;
                I1 i = this->xBegin_;
                I2 j = this->yBegin_;
                for (; i != this->xEnd_; ++i, ++j) {
                    error = value(*i) - *j;
                    totalError += error*error;
                }
                return totalError;
            }

            Real interpolationError() const {
                Real normalizedError = interpolationSquaredNonNormalizedError()
                    /(this->xEnd_-this->xBegin_);
                return std::sqrt(normalizedError);
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
