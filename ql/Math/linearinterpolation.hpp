
/*
 Copyright (C) 2000-2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file linearinterpolation.hpp
    \brief linear interpolation between discrete points
*/

#ifndef quantlib_linear_interpolation_hpp
#define quantlib_linear_interpolation_hpp

#include <ql/Math/interpolation.hpp>

namespace QuantLib {

    //! linear interpolation between discrete points
    class LinearInterpolation : public Interpolation {
      protected:
        //! linear interpolation implementation
        template <class I1, class I2>
        class Impl : public Interpolation::templateImpl<I1,I2> {
          public:
            Impl(const I1& xBegin, const I1& xEnd, const I2& yBegin)
            : Interpolation::templateImpl<I1,I2>(xBegin,xEnd,yBegin) {}
            double value(double x) const {
                Size i = locate(x);
                // I2 j = yBegin_+i;
                return yBegin_[i] + (x-xBegin_[i])*(yBegin_[i+1]-yBegin_[i])/
                                                   (xBegin_[i+1]-xBegin_[i]);
            }
            double primitive(double) const {
                QL_FAIL("LinearInterpolation::primitive(): "
                        "not implemented");
            }
            double derivative(double) const {
                QL_FAIL("LinearInterpolation::derivative(): "
                        "not implemented");
            }
            double secondDerivative(double) const {
                QL_FAIL("LinearInterpolation::secondDerivative(): "
                        "not implemented");
            }
        };
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        LinearInterpolation(const I1& xBegin, const I1& xEnd, 
                            const I2& yBegin) {
            impl_ = Handle<Interpolation::Impl>(
                  new LinearInterpolation::Impl<I1,I2>(xBegin, xEnd, yBegin));
        }
    };

}


#endif
