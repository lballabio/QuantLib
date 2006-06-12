/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mario Pucci

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
#include <ql/Utilities/null.hpp>
#include <vector>

namespace QuantLib {

    namespace detail {

        template <class I1, class I2> class SABRInterpolationImpl;

        class SABRCoefficientHolder {
          public:
            SABRCoefficientHolder() {}
            virtual ~SABRCoefficientHolder() {};
            Real t_, forward_, beta_, nu_, alpha_, rho_, oneMinusBeta_;
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
          public:
            SABRInterpolationImpl(const I1& xBegin, const I1& xEnd, const I2& yBegin,
                                  Time t,
                                  Real forward,
                                  Real beta,
                                  Real nu,
                                  Real alpha,
                                  Real rho)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              SABRCoefficientHolder() {

                QL_REQUIRE(t>0, "negative time not allowed");
                t_=t;

                forward_=forward;

                beta_=beta;
                nu_=nu;
                alpha_=alpha;
                rho_=rho;
                // fit any null parameters in the (beta_, nu_, alpha_, rho_) tuple
                calculate();
            }

            void calculate() {
                QL_REQUIRE(beta_!=Null<Real>(), "null beta not allowed"
                    "QuantLib is not ready to fit SABR yet!");
                QL_REQUIRE(nu_!=Null<Real>(), "null nu not allowed"
                    "QuantLib is not ready to fit SABR yet!");
                QL_REQUIRE(alpha_!=Null<Real>(), "null alpha not allowed"
                    "QuantLib is not ready to fit SABR yet!");
                QL_REQUIRE(rho_!=Null<Real>(), "null rho not allowed"
                    "QuantLib is not ready to fit SABR yet!");

                // fit goes here...

                QL_REQUIRE(beta_>=0.0 && beta_<=1.0, "beta must be in [0.0,1.0]");
                oneMinusBeta_ = 1.0-beta_;
                QL_REQUIRE(nu_>=0.0, "nu must be non negative");
                QL_REQUIRE(alpha_>0.0, "alpha must be positive");
                // ???
                //MFN_REQUIRE_1( rho_*rho_<1,"double Sabr::blackVolatilityByApproximation(...) const", rho_);

            }
            Real value(Real x) const {
                QL_REQUIRE(x>0.0, "negative value not allowed");
                const double A = std::pow(forward_*x, oneMinusBeta_);
                const double sqrtA= std::sqrt(A);
                const double logM = std::log(forward_/x);
                const double z = (nu_/alpha_)*sqrtA*logM;
                const double B = 1.0-2.0*rho_*z+z*z;
                const double C = oneMinusBeta_*oneMinusBeta_*logM*logM;
                const double tmp = (std::sqrt(B)+z-rho_)/(1.0-rho_);
                const double xx = std::log(tmp);
                const double D = sqrtA*(1.0+C/24.0+C*C/1920.0);
                const double d = 1.0 + t_ * (oneMinusBeta_*oneMinusBeta_*alpha_*alpha_/(24.0*A)
                    + 0.25*rho_*beta_*nu_*alpha_/sqrtA +(2.0-3.0*rho_*rho_)*(nu_*nu_/24));
                const double multiplier = (xx!=0.0 ? z/xx : 1.0);
                return (alpha_/D)*multiplier*d;;
            }
            Real primitive(Real x) const {
                return 0.0;
            }
            Real derivative(Real x) const {
                return 0.0;
            }
            Real secondDerivative(Real x) const {
                return 0.0;
            }
        };

    }

}

#endif
