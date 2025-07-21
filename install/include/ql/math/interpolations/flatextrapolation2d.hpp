/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti

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

/*! \file flatextrapolation2d.hpp
\brief abstract base classes for 2-D flat extrapolations
*/

#ifndef quantlib_flatextrapolation2D_hpp
#define quantlib_flatextrapolation2D_hpp

#include <ql/math/interpolations/interpolation2d.hpp>
#include <utility>

namespace QuantLib {
    
    /*! \ingroup interpolations
        \warning See the Interpolation class for information about the
                 required lifetime of the underlying data.
    */
    class FlatExtrapolator2D : public Interpolation2D {
      public:
        FlatExtrapolator2D(const ext::shared_ptr<Interpolation2D>& decoratedInterpolation) {
            impl_ = ext::shared_ptr<Interpolation2D::Impl>(
                  new FlatExtrapolator2DImpl(decoratedInterpolation));
        }
      protected:
       class FlatExtrapolator2DImpl: public Interpolation2D::Impl{
          public:
            FlatExtrapolator2DImpl(ext::shared_ptr<Interpolation2D> decoratedInterpolation)
            : decoratedInterp_(std::move(decoratedInterpolation)) {
                FlatExtrapolator2DImpl::calculate();
            }
            Real xMin() const override { return decoratedInterp_->xMin(); }
            Real xMax() const override { return decoratedInterp_->xMax(); }
            std::vector<Real> xValues() const override { return decoratedInterp_->xValues(); }
            Size locateX(Real x) const override { return decoratedInterp_->locateX(x); }
            Real yMin() const override { return decoratedInterp_->yMin(); }
            Real yMax() const override { return decoratedInterp_->yMax(); }
            std::vector<Real> yValues() const override { return decoratedInterp_->yValues(); }
            Size locateY(Real y) const override { return decoratedInterp_->locateY(y); }
            const Matrix& zData() const override { return decoratedInterp_->zData(); }
            bool isInRange(Real x, Real y) const override {
                return decoratedInterp_->isInRange(x,y);
            }
            void update() {
                decoratedInterp_->update();
            }
            void calculate() override {}
            Real value(Real x, Real y) const override {
                x = bindX(x);
                y = bindY(y);
                return (*decoratedInterp_)(x,y);
            }

          private:
            ext::shared_ptr<Interpolation2D> decoratedInterp_;

            Real bindX(Real x) const {
                if(x < xMin())
                    return xMin();
                if (x > xMax()) 
                    return xMax();
                return x;
            }
            Real bindY(Real y) const {
                if(y < yMin())
                    return yMin();
                if (y > yMax()) 
                    return yMax();
                return y;
            }

        };
    };


}


#endif
