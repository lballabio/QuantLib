/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Junjie Guo

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

/*! \file flatextrapolation.hpp
    \brief flat extrapolation decorator for 1-D interpolations
*/

#ifndef quantlib_flatextrapolation_hpp
#define quantlib_flatextrapolation_hpp

#include <ql/math/interpolation.hpp>
#include <utility>

namespace QuantLib {

    /*! \ingroup interpolations
        Decorator that turns any 1-D interpolation into one that
        extrapolates flat (i.e. clamping to the boundary values)
        outside the original data range.

        \warning See the Interpolation class for information about the
                 required lifetime of the underlying data.
    */
    class FlatExtrapolator : public Interpolation {
      public:
        explicit FlatExtrapolator(ext::shared_ptr<Interpolation> decoratedInterpolation) {
            impl_ = ext::make_shared<FlatExtrapolatorImpl>(
                std::move(decoratedInterpolation));
        }
      protected:
        class FlatExtrapolatorImpl final : public Interpolation::Impl {
          public:
            explicit FlatExtrapolatorImpl(ext::shared_ptr<Interpolation> decoratedInterpolation)
            : decoratedInterp_(std::move(decoratedInterpolation)) {
                FlatExtrapolatorImpl::calculate();
            }
            Real xMin() const override { return decoratedInterp_->xMin(); }
            Real xMax() const override { return decoratedInterp_->xMax(); }
            std::vector<Real> xValues() const override {
                return decoratedInterp_->xValues();
            }
            std::vector<Real> yValues() const override {
                return decoratedInterp_->yValues();
            }
            bool isInRange(Real x) const override {
                return decoratedInterp_->isInRange(x);
            }
            void update() override { decoratedInterp_->update(); }
            Real value(Real x) const override {
                return (*decoratedInterp_)(bind(x), true);
            }
            Real primitive(Real x) const override {
                if (x < xMin())
                    return decoratedInterp_->primitive(xMin(), true) +
                           (*decoratedInterp_)(xMin(), true) * (x - xMin());
                if (x > xMax())
                    return decoratedInterp_->primitive(xMax(), true) +
                           (*decoratedInterp_)(xMax(), true) * (x - xMax());
                return decoratedInterp_->primitive(x, true);
            }
            Real derivative(Real x) const override {
                if (x < xMin() || x > xMax())
                    return 0.0;
                return decoratedInterp_->derivative(x, true);
            }
            Real secondDerivative(Real x) const override {
                if (x < xMin() || x > xMax())
                    return 0.0;
                return decoratedInterp_->secondDerivative(x, true);
            }

          private:
            ext::shared_ptr<Interpolation> decoratedInterp_;

            Real bind(Real x) const {
                if (x < xMin())
                    return xMin();
                if (x > xMax())
                    return xMax();
                return x;
            }

            void calculate() { decoratedInterp_->update(); }
        };
    };

}

#endif
