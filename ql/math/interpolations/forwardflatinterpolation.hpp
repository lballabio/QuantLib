/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2008 StatPro Italia srl

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

/*! \file forwardflatinterpolation.hpp
    \brief forward-flat interpolation between discrete points
*/

#ifndef quantlib_forward_flat_interpolation_hpp
#define quantlib_forward_flat_interpolation_hpp

#include <ql/math/interpolation.hpp>
#include <vector>

namespace QuantLib {

    namespace detail {
        template<class I1, class I2> class ForwardFlatInterpolationImpl;
    }

    //! Forward-flat interpolation between discrete points
    /*! \ingroup interpolations
        \warning See the Interpolation class for information about the
                 required lifetime of the underlying data.
    */
    class ForwardFlatInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        ForwardFlatInterpolation(const I1& xBegin, const I1& xEnd,
                                 const I2& yBegin) {
            impl_ = ext::shared_ptr<Interpolation::Impl>(new
                detail::ForwardFlatInterpolationImpl<I1,I2>(xBegin, xEnd,
                                                            yBegin));
            impl_->update();
        }
    };

    //! Forward-flat interpolation factory and traits
    /*! \ingroup interpolations */
    class ForwardFlat {
      public:
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return ForwardFlatInterpolation(xBegin, xEnd, yBegin);
        }
        static const bool global = false;
        static const Size requiredPoints = 2;
    };

    namespace detail {

        template <class I1, class I2>
        class ForwardFlatInterpolationImpl
            : public Interpolation::templateImpl<I1,I2> {
          public:
            ForwardFlatInterpolationImpl(const I1& xBegin, const I1& xEnd,
                                         const I2& yBegin)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin,
                                                 ForwardFlat::requiredPoints),
              primitive_(xEnd-xBegin), n_(xEnd-xBegin) {}
            void update() override {
                primitive_[0] = 0.0;
                for (Size i=1; i<n_; i++) {
                    Real dx = this->xBegin_[i]-this->xBegin_[i-1];
                    primitive_[i] = primitive_[i-1] + dx*this->yBegin_[i-1];
                }
            }
            Real value(Real x) const override {
                if (x >= this->xBegin_[n_-1])
                    return this->yBegin_[n_-1];

                Size i = this->locate(x);
                return this->yBegin_[i];
            }
            Real primitive(Real x) const override {
                Size i = this->locate(x);
                Real dx = x-this->xBegin_[i];
                return primitive_[i] + dx*this->yBegin_[i];
            }
            Real derivative(Real) const override { return 0.0; }
            Real secondDerivative(Real) const override { return 0.0; }

          private:
            std::vector<Real> primitive_;
            Size n_;
        };

    }

}

#endif


#ifndef id_596a8d11878f3ed5e6fcc4c8110da79f
#define id_596a8d11878f3ed5e6fcc4c8110da79f
inline bool test_596a8d11878f3ed5e6fcc4c8110da79f(int* i) { return i != 0; }
#endif
