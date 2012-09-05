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

/*! \file 
\brief abstract base classes for 2-D flat extrapolations
*/

#ifndef quantlib_flatextrapolation2D_hpp
#define quantlib_flatextrapolation2D_hpp

#include <ql/math/interpolations/interpolation2d.hpp>

namespace QuantLib {

    
    class FlatExtrapolator2D : public Interpolation2D {
      public:
        FlatExtrapolator2D(boost::shared_ptr<Interpolation2D> decoratedInterpolation) {
            impl_ = boost::shared_ptr<Interpolation2D::Impl>(
                  new FlatExtrapolator2DImpl(decoratedInterpolation));
        }
      protected:
       class FlatExtrapolator2DImpl: public Interpolation2D::Impl{
          public:
            FlatExtrapolator2DImpl(boost::shared_ptr<Interpolation2D> decoratedInterpolation)
            :decoratedInterp_(decoratedInterpolation){
                calculate();
            }
            Real xMin() const {
                return decoratedInterp_->xMin();
            }
            Real xMax() const {
                return decoratedInterp_->xMax();
            }
            std::vector<Real> xValues() const {
                return decoratedInterp_->xValues();
            }
            Size locateX(Real x) const {
                return decoratedInterp_->locateX(x);
            }
            Real yMin() const {
                return decoratedInterp_->yMin();
            }
            Real yMax() const {
                return decoratedInterp_->yMax();
            }
            std::vector<Real> yValues() const {
                return decoratedInterp_->yValues();
            }
            Size locateY(Real y) const {
                return decoratedInterp_->locateY(y);
            }
            const Matrix& zData() const {
                return decoratedInterp_->zData();
            }
            bool isInRange(Real x, Real y) const {
                return decoratedInterp_->isInRange(x,y);
            }
            void update() {
                decoratedInterp_->update();
            }
            void calculate() {}
            Real value(Real x, Real y) const {
                x = bindX(x);
                y = bindY(y);
                return decoratedInterp_->operator()(x,y);
            }

          private:
            boost::shared_ptr<Interpolation2D> decoratedInterp_;

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
