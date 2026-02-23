/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2008 StatPro Italia srl

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

/*! \file backwardflatinterpolation.hpp
    \brief backward-flat interpolation between discrete points
*/

#ifndef quantlib_backward_flat_interpolation_hpp
#define quantlib_backward_flat_interpolation_hpp

#include <ql/math/interpolation.hpp>
#include <vector>

namespace QuantLib {

    namespace detail {
        template<class I1, class I2> class BackwardFlatInterpolationImpl;
    }

    //! Backward-flat interpolation between discrete points
    /*! \ingroup interpolations
        \warning See the Interpolation class for information about the
                 required lifetime of the underlying data.
    */
    class BackwardFlatInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        BackwardFlatInterpolation(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) {
            impl_ = ext::shared_ptr<Interpolation::Impl>(new
                detail::BackwardFlatInterpolationImpl<I1,I2>(xBegin, xEnd,
                                                             yBegin));
            impl_->update();
        }
    };

    //! Backward-flat interpolation factory and traits
    /*! \ingroup interpolations */
    class BackwardFlat {
      public:
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return BackwardFlatInterpolation(xBegin, xEnd, yBegin);
        }
        static const bool global = false;
        static const Size requiredPoints = 1;
    };

    namespace detail {

        template <class I1, class I2>
        class BackwardFlatInterpolationImpl final
            : public Interpolation::templateImpl<I1,I2> {
          public:
            BackwardFlatInterpolationImpl(const I1& xBegin, const I1& xEnd,
                                          const I2& yBegin)
            : Interpolation::templateImpl<I1,I2>(xBegin,xEnd,yBegin,
                                                 BackwardFlat::requiredPoints),
              primitive_(xEnd-xBegin) {}
            void update() override {
                Size n = this->xEnd_-this->xBegin_;
                primitive_[0] = 0.0;
                for (Size i=1; i<n; i++) {
                    Real dx = this->xBegin_[i]-this->xBegin_[i-1];
                    primitive_[i] = primitive_[i-1] + dx*this->yBegin_[i];
                }
            }
            Real value(Real x) const override {
                if (x <= this->xBegin_[0]
                    || std::distance(this->xBegin_, this->xEnd_) == 1)
                    return this->yBegin_[0];

                Size i = this->locate(x);
                if (x == this->xBegin_[i])
                    return this->yBegin_[i];
                else
                    return this->yBegin_[i+1];
            }
            Real primitive(Real x) const override {
                if (std::distance(this->xBegin_, this->xEnd_) == 1)
                    return (x - this->xBegin_[0]) * this->yBegin_[0];

                Size i = this->locate(x);
                Real dx = x-this->xBegin_[i];
                return primitive_[i] + dx*this->yBegin_[i+1];
            }
            Real derivative(Real) const override { return 0.0; }
            Real secondDerivative(Real) const override { return 0.0; }

          private:
            std::vector<Real> primitive_;
        };

    }

}

#endif
