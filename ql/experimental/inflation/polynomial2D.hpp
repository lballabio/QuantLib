/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) Berndt Engelmann 2009

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

/*! \file polynomial2D.hpp
    \brief quadratic polynomial interpolation between discrete points
*/

#ifndef quantlib_polynomial2D_interpolation_hpp
#define quantlib_polynomial2D_interpolation_hpp

#include <ql/math/interpolation.hpp>
#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>
#include <ql/utilities/null.hpp>
#include <vector>

namespace QuantLib {

    namespace detail2 {

        class CoefficientHolder {
          public:
            CoefficientHolder(Size n) : n_(n), primitiveConst_(n-1), a_(n-1), b_(n-1), c_(n-1) {}
            virtual ~CoefficientHolder() {}
            Size n_;
            // P[i](x) = a[i] +
            //           b[i]*(x-x[i]) +
            //           c[i]*(x-x[i])^2
            std::vector<Real> primitiveConst_, a_, b_, c_;
        };

        template <class I1, class I2> class Polynomial2DInterpolationImpl;
    }

    //! %Quadratic polynomial interpolation between discrete points
    class Polynomial2DInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        Polynomial2DInterpolation(const I1& xBegin, const I1& xEnd, const I2& yBegin) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(new
                detail2::Polynomial2DInterpolationImpl<I1,I2>(xBegin, xEnd, yBegin));
            impl_->update();
            coeffs_ = boost::dynamic_pointer_cast<detail2::CoefficientHolder>(impl_);
        }
        const std::vector<Real>& primitiveConstants() const {
            return coeffs_->primitiveConst_;
        }
        const std::vector<Real>& aCoefficients() const { return coeffs_->a_; }
        const std::vector<Real>& bCoefficients() const { return coeffs_->b_; }
        const std::vector<Real>& cCoefficients() const { return coeffs_->c_; }

	  private:
	    boost::shared_ptr<detail2::CoefficientHolder> coeffs_;
    };

    //! %Quadratic polynomial interpolation factory and traits
    class Polynomial2D {
      public:
        Polynomial2D() {}
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return Polynomial2DInterpolation(xBegin, xEnd, yBegin);
        }
        enum { global = 1 };
        enum { requiredPoints = 3 };
    };


    namespace detail2 {

        template <class I1, class I2>
        class Polynomial2DInterpolationImpl : public Interpolation::templateImpl<I1,I2>,
                                public CoefficientHolder {
          public:
            Polynomial2DInterpolationImpl(const I1& xBegin, const I1& xEnd, const I2& yBegin)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              CoefficientHolder(xEnd-xBegin) {}

            void update() {
			  QL_REQUIRE(n_ >= Polynomial2D::requiredPoints,
			    "polynomial interpolation is impossible with less than three points");

              size_t i;
			  primitiveConst_[0] = 0.0;
			  for (i=0; i<n_-2; ++i) {
                // fit a quadratic polynomial to the points i, i+1, i+2
                // form of the polynomial a + b*x + c*x*x
                double x1 = this->xBegin_[i];
				double x2 = this->xBegin_[i+1];
                double x3 = this->xBegin_[i+2];
				double y1 = this->yBegin_[i];
				double y2 = this->yBegin_[i+1];
				double y3 = this->yBegin_[i+2];
				double denom = (x1-x2)*(x1-x3)*(x2-x3);
                double a = (x1*(x1-x3)*x3*y2 + x2*x2*(x1*y3-x3*y1) + x2*(x3*x3*y1-x1*x1*y3)) / (-denom);
				double b = (x3*x3*(y1-y2) + x1*x1*(y2-y3) + x2*x2*(y3-y1)) / denom;
				double c = (x3*(y2-y1) + x2*(y1-y3) + x1*(y3-y2)) / denom;
				a_[i] = a;
				b_[i] = b;
				c_[i] = c;
				primitiveConst_[i+1] = primitiveConst_[i]
				                     + x2 * (a_[i] + x2 * (b_[i] / 2.0 + x2 * c_[i] / 3.0))
									 - x1 * (a_[i] + x1 * (b_[i] / 2.0 + x1 * c_[i] / 3.0));
			  }
			  a_[n_-2] = a_[n_-3];
			  b_[n_-2] = b_[n_-3];
			  c_[n_-2] = c_[n_-3];
			
			}
            Real value(Real x) const {
                Size j = this->locate(x);
                return a_[j] + x * (b_[j] + x*c_[j]);
            }
            Real primitive(Real x) const {
                Size j = this->locate(x);
                Real xStart = this->xBegin_[j];
                return primitiveConst_[j]
					+ x * (a_[j] + x * (b_[j] / 2.0 + x * c_[j] / 3.0))
				    - xStart * (a_[j] + xStart * (b_[j] / 2.0 + xStart * c_[j] / 3.0));
            }
            Real derivative(Real x) const {
                Size j = this->locate(x);
                return b_[j] + 2.0*c_[j]*x;
            }
            Real secondDerivative(Real x) const {
                Size j = this->locate(x);
                return 2.0*c_[j];
            }
        };

    }

}

#endif
