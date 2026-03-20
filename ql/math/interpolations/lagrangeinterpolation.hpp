/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

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

#ifndef quantlib_lagrange_interpolation_hpp
#define quantlib_lagrange_interpolation_hpp

#include <ql/math/array.hpp>
#include <ql/math/interpolation.hpp>
#if defined(QL_EXTRA_SAFETY_CHECKS)
#include <set>
#endif

namespace QuantLib {
    /*! References: J-P. Berrut and L.N. Trefethen,
                    Barycentric Lagrange interpolation,
                    SIAM Review, 46(3):501–517, 2004.
        https://people.maths.ox.ac.uk/trefethen/barycentric.pdf
    */

    namespace detail {
        class UpdatedYInterpolation {
          public:
            virtual ~UpdatedYInterpolation() = default;
            virtual Real value(const Array& yValues, Real x) const = 0;
        };

        template <class I1, class I2>
        class LagrangeInterpolationImpl final
            : public Interpolation::templateImpl<I1,I2>,
              public UpdatedYInterpolation {

          public:
            LagrangeInterpolationImpl(const I1& xBegin, const I1& xEnd,
                                      const I2& yBegin)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              n_(std::distance(xBegin, xEnd)),
              lambda_(n_) {
                #if defined(QL_EXTRA_SAFETY_CHECKS)
                QL_REQUIRE(std::set<Real>(xBegin, xEnd).size() == n_,
                        "x values must not contain duplicates");
                #endif
            }

            void update() override {
                const Real cM1 = 4.0/(*(this->xEnd_-1) - *(this->xBegin_));

                for (Size i=0; i < n_; ++i) {
                    lambda_[i] = 1.0;

                    const Real x_i = this->xBegin_[i];
                    for (Size j=0; j < n_; ++j) {
                        if (i != j)
                            lambda_[i] *= cM1*(x_i-this->xBegin_[j]);
                    }
                    lambda_[i] = 1.0/lambda_[i];
                }
            }

            Real value(Real x) const override { return _value(this->yBegin_, x); }

            Real derivative(Real x) const override {
                Real n=0.0, d=0.0, nd=0.0, dd=0.0;
                for (Size i=0; i < n_; ++i) {
                    const Real x_i = this->xBegin_[i];

                    if (close_enough(x, x_i)) {
                        Real p=0.0;
                        for (Size j=0; j < n_; ++j)
                            if (i != j) {
                                p+=lambda_[j]/(x-this->xBegin_[j])
                                    *(this->yBegin_[j] - this->yBegin_[i]);
                            }
                        return p/lambda_[i];
                    }

                    const Real alpha = lambda_[i]/(x-x_i);
                    const Real alphad = -alpha/(x-x_i);
                    n += alpha * this->yBegin_[i];
                    d += alpha;
                    nd += alphad * this->yBegin_[i];
                    dd += alphad;
                }
                return (nd * d - n * dd)/(d*d);
            }

            Real primitive(Real) const override {
                QL_FAIL("LagrangeInterpolation primitive is not implemented");
            }

            Real secondDerivative(Real) const override {
                QL_FAIL("LagrangeInterpolation secondDerivative "
                        "is not implemented");
            }

            Real value(const Array& y, Real x) const override { return _value(y.begin(), x); }

          private:
            template <class Iterator>
            Real _value(const Iterator& yBegin, Real x) const {

                const Real eps = 10*QL_EPSILON*std::abs(x);
                const auto iter = std::lower_bound(
                    this->xBegin_, this->xEnd_, x - eps);
                if (iter != this->xEnd_ && *iter - x < eps) {
                    return yBegin[std::distance(this->xBegin_, iter)];
                }

                Real n = 0.0, d = 0.0;
                for (Size i = 0; i < n_; ++i) {
                    const Real alpha = lambda_[i] / (x - this->xBegin_[i]);
                    n += alpha * yBegin[i];
                    d += alpha;
                }
                return n / d;
              }

              const Size n_;
              Array lambda_;
        };
    }

    /*! \ingroup interpolations
        \warning See the Interpolation class for information about the
                 required lifetime of the underlying data.
    */
    class LagrangeInterpolation : public Interpolation {
      public:
        template <class I1, class I2>
        LagrangeInterpolation(const I1& xBegin, const I1& xEnd,
                              const I2& yBegin) {
            impl_ = ext::make_shared<detail::LagrangeInterpolationImpl<I1,I2> >(
                xBegin, xEnd, yBegin);
            impl_->update();
        }

        // interpolate with new set of y values for a new x value
        Real value(const Array& y, Real x) const {
            return ext::dynamic_pointer_cast<detail::UpdatedYInterpolation>
                (impl_)->value(y, x);
        }
    };

}

#endif
