/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2008 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

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

/*! \file constrainedcubicspline.hpp
    \brief constrained cubic spline interpolation between discrete points
*/

#ifndef quantlib_constrained_cubic_spline_hpp
#define quantlib_constrained_cubic_spline_hpp

#include <ql/math/interpolation.hpp>
#include <vector>

namespace QuantLib {

    namespace detail {

        class CoefficientHolder2 {
          public:
            CoefficientHolder2(Size n)
            : n_(n), primitiveConst_(n-1), a_(n-1), b_(n-1), c_(n-1) {}
            virtual ~CoefficientHolder2() {}
            Size n_;
            // P[i](x) = y[i] +
            //           a[i]*(x-x[i]) +
            //           b[i]*(x-x[i])^2 +
            //           c[i]*(x-x[i])^3
            std::vector<Real> primitiveConst_, a_, b_, c_;
        };

        template <class I1, class I2> class ConstrainedCubicSplineInterpolationImpl;

    }

    //! %Costrained spline interpolation between discrete points.
    /*! add here references
    */
    class ConstrainedCubicSplineInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        ConstrainedCubicSplineInterpolation(const I1& xBegin,
                                            const I1& xEnd,
                                            const I2& yBegin) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(new
                detail::ConstrainedCubicSplineInterpolationImpl<I1,I2>(
                                                        xBegin, xEnd, yBegin));
            impl_->update();
            coeffs_ =
                boost::dynamic_pointer_cast<detail::CoefficientHolder2>(impl_);
        }
        const std::vector<Real>& primitiveConstants() const {
            return coeffs_->primitiveConst_;
        }
        const std::vector<Real>& aCoefficients() const { return coeffs_->a_; }
        const std::vector<Real>& bCoefficients() const { return coeffs_->b_; }
        const std::vector<Real>& cCoefficients() const { return coeffs_->c_; }
      private:
        boost::shared_ptr<detail::CoefficientHolder2> coeffs_;
    };


    //! %Cubic spline interpolation factory and traits
    class ConstrainedCubicSpline {
      public:
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin,
                                  const I1& xEnd,
                                  const I2& yBegin) const {
            return ConstrainedCubicSplineInterpolation(xBegin, xEnd, yBegin);
        }
        static const bool global = true;
        static const Size requiredPoints = 2;
    };


    namespace detail {

        template <class I1, class I2>
        class ConstrainedCubicSplineInterpolationImpl : public CoefficientHolder2,
                                    public Interpolation::templateImpl<I1,I2> {
          public:
            ConstrainedCubicSplineInterpolationImpl(const I1& xBegin,
                                                    const I1& xEnd,
                                                    const I2& yBegin)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              CoefficientHolder2(xEnd-xBegin) {}

            void update() {

                std::vector<Real> tmp(n_), dx(n_-1), S(n_-1);

                for (Size i=0; i<n_-1; ++i) {
                    dx[i] = this->xBegin_[i+1] - this->xBegin_[i];
                    S[i] = (this->yBegin_[i+1] - this->yBegin_[i])/dx[i];
                }

                if (n_==2)
                    tmp[0] = tmp[1] = S[0];
                else {
                    // intermediate points
                    for (Size i=1; i<n_-1; ++i) {
                        if (S[i-1]*S[i]<0.0)
                            // slope changes sign at point
                            tmp[i] = 0.0;
                        else
                            // slope will be between the slopes of the adjacent
                            // straight lines and should approach zero if the
                            // slope of either line approaches zero
                            tmp[i] = 2.0/(1.0/S[i-1]+1.0/S[i]);
                    }
                    // end points
                    tmp[0] = (3.0*S[0]-tmp[1])/2.0;
                    tmp[n_-1] = (3.0*S[n_-2]-tmp[n_-2])/2.0;
                }

                for (Size i=0; i<n_-1; ++i) {
                    a_[i] = tmp[i];
                    b_[i] = (3.0*S[i] - tmp[i+1] - 2.0*tmp[i])/dx[i];
                    c_[i] = (tmp[i+1] + tmp[i] - 2.0*S[i])/(dx[i]*dx[i]);
                }

                primitiveConst_[0] = 0.0;
                for (Size i=1; i<n_-1; ++i) {
                    primitiveConst_[i] = primitiveConst_[i-1]
                        + dx[i-1] *
                        (this->yBegin_[i-1] + dx[i-1] *
                         (a_[i-1]/2.0 + dx[i-1] *
                          (b_[i-1]/3.0 + dx[i-1] * c_[i-1]/4.0)));
                }
            }
            Real value(Real x) const {
                Size j = this->locate(x);
                Real dx = x-this->xBegin_[j];
                return this->yBegin_[j] + dx*(a_[j] + dx*(b_[j] + dx*c_[j]));
            }
            Real primitive(Real x) const {
                Size j = this->locate(x);
                Real dx = x-this->xBegin_[j];
                return primitiveConst_[j]
                    + dx*(this->yBegin_[j] + dx*(a_[j]/2.0
                    + dx*(b_[j]/3.0 + dx*c_[j]/4.0)));
            }
            Real derivative(Real x) const {
                Size j = this->locate(x);
                Real dx = x-this->xBegin_[j];
                return a_[j] + (2.0*b_[j] + 3.0*c_[j]*dx)*dx;
            }
            Real secondDerivative(Real x) const {
                Size j = this->locate(x);
                Real dx = x-this->xBegin_[j];
                return 2.0*b_[j] + 6.0*c_[j]*dx;
            }
        };

    }

}

#endif
