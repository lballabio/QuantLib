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
#include <ql/Optimization/problem.hpp>
#include <ql/Optimization/conjugategradient.hpp>
#include <ql/Utilities/null.hpp>
#include <vector>

namespace QuantLib {

    namespace detail {

        template <class I1, class I2> class SABRInterpolationImpl;

        class SABRCoefficientHolder {
          public:
            SABRCoefficientHolder(Time t,
                                  Real forward,
                                  Real beta,
                                  Real nu,
                                  Real alpha,
                                  Real rho)
            : t_(t), forward_(forward), beta_(beta), nu_(nu),
              alpha_(alpha), rho_(rho), fixed_(4) {

                QL_REQUIRE(t>0, "negative time not allowed");

                fixed_[0] = (beta_ != Null<Real>());
                fixed_[1] = (nu_ != Null<Real>());
                fixed_[2] = (alpha_ != Null<Real>());
                fixed_[3] = (rho_ != Null<Real>());
            }
            virtual ~SABRCoefficientHolder() {}
            Real t_, forward_, beta_, nu_, alpha_, rho_;
            std::vector<bool> fixed_;
        };

    }

    //! %SABR interpolation between discrete points.
    /*! Blah blah.

        See XXX
    */
    class SABRInterpolation : public Interpolation {
      public:
        /*! */
        template <class I1, class I2>
        SABRInterpolation(const I1& xBegin,
                          const I1& xEnd,
                          const I2& yBegin,
                          Time t,
                          Real forward,
                          Real beta,
                          Real nu,
                          Real alpha,
                          Real rho) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(
                        new detail::SABRInterpolationImpl<I1,I2>(
                            xBegin, xEnd, yBegin,
                            t, forward, beta, nu, alpha, rho));
            coeffs_ =
                boost::dynamic_pointer_cast<detail::SABRCoefficientHolder>(impl_);
        }
        Real expiry() const {
            return coeffs_->t_;
        }
        Real forward() const {
            return coeffs_->forward_;
        }
        Real beta() const {
            return coeffs_->beta_;
        }
        Real nu() const {
            return coeffs_->nu_;
        }
        Real alpha() const {
            return coeffs_->alpha_;
        }
        Real rho() const {
            return coeffs_->rho_;
        }
      private:
        boost::shared_ptr<detail::SABRCoefficientHolder> coeffs_;
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
                    if (!sabr_->fixed_[0]) sabr_->beta_ = x[0];
                    if (!sabr_->fixed_[1]) sabr_->nu_ = x[1];
                    if (!sabr_->fixed_[2]) sabr_->alpha_ = x[2];
                    if (!sabr_->fixed_[3]) sabr_->rho_ = x[3];

                    Real error, totalError = 0.0;
                    I1 i = sabr_->xBegin_, xEnd = sabr_->xEnd_;
                    I2 j = sabr_->yBegin_;
                    for (; i != xEnd; ++i, ++j) {
                        error = sabr_->value(*i) - *j;
                        totalError += error*error;
                    }
                    return totalError;
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
                        return params[0]>=0.0 && params[0]<=1.0 // beta
                            && params[1]>=0.0                   // nu
                            && params[2]>0.0                    // alpha
                            && params[3]*params[3] < 1.0;       // rho
                    }
                };
              public:
                SABRConstraint()
                : Constraint(boost::shared_ptr<Constraint::Impl>(
                                             new SABRConstraint::Impl)) {}
            };

          public:
            SABRInterpolationImpl(const I1& xBegin, const I1& xEnd, const I2& yBegin,
                                  Time t,
                                  Real forward,
                                  Real beta,
                                  Real nu,
                                  Real alpha,
                                  Real rho)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              SABRCoefficientHolder(t, forward, beta, nu, alpha, rho) {

                // fit any null parameters in the (beta_, nu_, alpha_, rho_) tuple
                calculate();
            }

            void calculate() {
                if (beta_ != Null<Real>())
                    QL_REQUIRE(beta_>=0.0 && beta_<=1.0,
                               "beta must be in [0.0,1.0]");
                if (nu_ != Null<Real>())
                    QL_REQUIRE(nu_>=0.0, "nu must be non negative");
                if (alpha_ != Null<Real>())
                    QL_REQUIRE(alpha_>0.0, "alpha must be positive");
                if (rho_ != Null<Real>())
                    QL_REQUIRE(rho_*rho_<1,"rho square must be less than 1");

                if (beta_ != Null<Real>() && nu_ != Null<Real>()
                    && alpha_ != Null<Real>() && rho_ != Null<Real>())
                    return;

                SABRConstraint constraint;
                SABRError costFunction(this);
                ConjugateGradient method;
                Array guess(4);
                guess[0] = 0.40; // beta
                guess[1] = 0.36; // nu
                guess[2] = 0.02; // alpha
                guess[3] = 0.20; // rho
                method.setInitialValue(guess);
                method.setEndCriteria(EndCriteria(3000,1e-12));
                method.endCriteria().setPositiveOptimization();

                Problem problem(costFunction, constraint, method);
                problem.minimize();

                Array result = problem.minimumValue();
                if (!fixed_[0]) beta_ = result[0];
                if (!fixed_[1]) nu_ = result[1];
                if (!fixed_[2]) alpha_ = result[2];
                if (!fixed_[3]) rho_ = result[3];

                QL_ENSURE(beta_>=0.0 && beta_<=1.0,
                          "beta must be in [0.0,1.0]");
                QL_ENSURE(nu_>=0.0, "nu must be non negative");
                QL_ENSURE(alpha_>0.0, "alpha must be positive");
                QL_ENSURE(rho_*rho_<1,"rho square must be less than 1");
            }

            Real value(Real x) const {
                QL_REQUIRE(x>0.0, "negative value not allowed");
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
                const Real d = 1.0 + t_ * (oneMinusBeta*oneMinusBeta*alpha_*alpha_/(24.0*A)
                    + 0.25*rho_*beta_*nu_*alpha_/sqrtA +(2.0-3.0*rho_*rho_)*(nu_*nu_/24));
                const Real multiplier = (xx!=0.0 ? z/xx : 1.0);
                return (alpha_/D)*multiplier*d;;
            }
            Real primitive(Real x) const {
                QL_FAIL("not implemented");
            }
            Real derivative(Real x) const {
                QL_FAIL("not implemented");
            }
            Real secondDerivative(Real x) const {
                QL_FAIL("not implemented");
            }
        };

    }

}

#endif
