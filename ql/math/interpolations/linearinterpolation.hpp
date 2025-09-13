/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2008 StatPro Italia srl

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

/*! \file linearinterpolation.hpp
    \brief linear interpolation between discrete points
*/

#ifndef quantlib_linear_interpolation_hpp
#define quantlib_linear_interpolation_hpp

#include <ql/math/interpolation.hpp>
#include <vector>

namespace QuantLib {

    namespace detail {
        template<class I1, class I2> class LinearInterpolationImpl;
    }

    //! %Linear interpolation between discrete points
    /*! \ingroup interpolations
        \warning See the Interpolation class for information about the
                 required lifetime of the underlying data.
    */
    class LinearInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        LinearInterpolation(const I1& xBegin, const I1& xEnd,
                            const I2& yBegin) {
            impl_ = ext::shared_ptr<Interpolation::Impl>(new
                detail::LinearInterpolationImpl<I1,I2>(xBegin, xEnd,
                                                       yBegin));
            impl_->update();
        }
    };

    //! %Linear-interpolation factory and traits
    /*! \ingroup interpolations */
    class Linear {
      public:
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return LinearInterpolation(xBegin, xEnd, yBegin);
        }
        static const bool global = false;
        static const Size requiredPoints = 2;
    };

    namespace detail {

        template <class I1, class I2>
        class LinearInterpolationImpl
            : public Interpolation::templateImpl<I1,I2> {
          public:
            LinearInterpolationImpl(const I1& xBegin, const I1& xEnd,
                                    const I2& yBegin)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin,
                                                 Linear::requiredPoints),
              primitiveConst_(xEnd-xBegin), s_(xEnd-xBegin) {}
            void update() override {
                primitiveConst_[0] = 0.0;
                for (Size i=1; i<Size(this->xEnd_-this->xBegin_); ++i) {
                    Real dx = this->xBegin_[i]-this->xBegin_[i-1];
                    s_[i-1] = (Real(this->yBegin_[i])-Real(this->yBegin_[i-1]))/dx;
                    primitiveConst_[i] = primitiveConst_[i-1]
                        + dx*(this->yBegin_[i-1] +0.5*dx*s_[i-1]);
                }
            }
            Real value(Real x) const override {
                Size i = this->locate(x);
                return this->yBegin_[i] + (x-this->xBegin_[i])*s_[i];
            }
            Real primitive(Real x) const override {
                Size i = this->locate(x);
                Real dx = x-this->xBegin_[i];
                return primitiveConst_[i] +
                    dx*(this->yBegin_[i] + 0.5*dx*s_[i]);
            }
            Real derivative(Real x) const override {
                Size i = this->locate(x);
                return s_[i];
            }
            Real secondDerivative(Real) const override { return 0.0; }

          private:
            std::vector<Real> primitiveConst_, s_;
        };

    }

}

#endif
