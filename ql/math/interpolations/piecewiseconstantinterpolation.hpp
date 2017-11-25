/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Klaus Spanderen

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

/*! \file piecewiseconstantinterpolation.hpp
    \brief piecewise constant interpolation
*/

#ifndef quantlib_piecewise_constant_interpolation_hpp
#define quantlib_piecewise_constant_interpolation_hpp

#include <ql/math/interpolation.hpp>

#include <numeric>

namespace QuantLib {
    namespace detail {
        template<class I1, class I2>
        class PiecewiseConstantInterpolationImpl;
    }

    class PiecewiseConstantInterpolation : public Interpolation {
      public:
        template <class I1, class I2>
        PiecewiseConstantInterpolation(
            const I1& xBegin, const I1& xEnd, const I2& yBegin) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(new
                detail::PiecewiseConstantInterpolationImpl<I1,I2>(
                    xBegin, xEnd, yBegin));
        }
    };

    class PiecewiseConstant {
      public:
        template <class I1, class I2>
        Interpolation interpolate(
            const I1& xBegin, const I1& xEnd, const I2& yBegin) const {
            return PiecewiseConstantInterpolation(xBegin, xEnd, yBegin);
        }
    };

    namespace detail {

        template <class I1, class I2>
        class PiecewiseConstantInterpolationImpl
            : public Interpolation::templateImpl<I1,I2> {
          public:
            PiecewiseConstantInterpolationImpl(
                const I1& xBegin, const I1& xEnd, const I2& yBegin)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin, 1) { }

            void update() { }

            Real value(Real x) const {
                return this->yBegin_[
                    std::min<std::size_t>(
                        std::distance(
                            this->xBegin_,
                            std::upper_bound(this->xBegin_, this->xEnd_, x)),
                        std::distance(this->xBegin_, this->xEnd_) - 1)];
            }

            Real primitive(Real x) const {
                if (   x <= *(this->xBegin_)
                    || std::distance(this->xBegin_, this->xEnd_) == 1)
                    return (x - this->xBegin_[0]) * this->yBegin_[0];

                const std::size_t idx = std::distance(
                    this->xBegin_,
                    std::upper_bound(this->xBegin_, this->xEnd_, x)) - 1;

                Real sum = 0.0;
                for (Size i=0; i < idx; ++i)
                    sum += (this->xBegin_[i+1] - this->xBegin_[i])
                        * this->yBegin_[i+1];

                sum += (x - this->xBegin_[idx])
                    * this->yBegin_[std::min<std::size_t>(idx+1,
                        std::distance(this->xBegin_, this->xEnd_)-1)];

                return sum;
            }

            Real derivative(Real x) const {
                if (isKnotePoint(x))
                    return std::numeric_limits<Real>::quiet_NaN();
                else
                    return 0.0;
            }

            Real secondDerivative(Real x) const {
                return derivative(x);
            }

          private:
              bool isKnotePoint(Real x) const {
                  return std::find(this->xBegin_, this->xEnd_, x)
                      != this->xEnd_;
              }
        };
    }
}
#endif
